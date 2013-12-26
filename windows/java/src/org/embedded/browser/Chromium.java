package org.embedded.browser;

import java.util.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.widgets.*;

public class Chromium extends Canvas {
	static volatile boolean inited = false;
	static int count = 0;
	static HashMap<Integer, Chromium> chmap = new HashMap<Integer, Chromium>();
	int id, hwnd, chptr;
	boolean tab = false, webpage_loading = false;
	boolean can_go_back = false, can_go_forward = false;
	volatile boolean loaded = false;
	ChromeSettings chromeset;
	Thread init;
	Composite parentbak;
	TitleChangeListener tcl = null;
	NewWindowListener nwl = null;
	CloseWindowListener cwl = null;
	LoadStatusListener lsl = null;
	NavStateListener nsl = null;
	
	public Chromium (Composite parent, int style) {
		this(parent, style, "about:blank");
	}
	
	public Chromium (Composite parent, int style, String url) {
		this(parent, style, url, new ChromeSettings());
	}
	
	public Chromium (Composite parent, int style, final String url, ChromeSettings chromesettings) {
		super(parent, style);
		parentbak = parent;
		chromeset = chromesettings.Finalize();
	  synchronized (chmap) {
		hwnd = (int) this.handle;
		id = ++count;
		chmap.put(id, this);
		if (inited) {
			tab = true;
			browser_new(hwnd, id, url, chromeset);
		}
		else {
			init = new Thread(new Runnable() {
				public void run() {
					browser_init(hwnd, url, chromeset);
				}
			});
			init.start();
			while (!inited) { try {
				Thread.sleep(3);
			  } catch (InterruptedException e) { }
			}
		}
		this.addDisposeListener(new DisposeListener() {
			public void widgetDisposed(DisposeEvent e) {
				synchronized (chmap) {
					//System.out.println(id);
					chmap.remove(id);
					if (tab)
						browser_close(chptr);
					else
						browser_shutdown(chptr);
				}
			}
		});
		this.addControlListener(new ControlListener() {
			public void controlMoved(ControlEvent arg0) { }
			public void controlResized(ControlEvent arg0) {
				browser_resized(chptr, hwnd);
			}
		});
	  }
	}
	
	public void setUrl(final String url) {
		if (!loaded) {
			Thread su = new Thread(new Runnable() {
				public void run() {
					while (!loaded) { try { Thread.sleep(3); } catch (InterruptedException e) { } }
					browser_setUrl(chptr, url);
				}
			});
			su.start();
		}
		else
			browser_setUrl(chptr, url);
	}
	
	public void cleanCookies() {
		browser_clean_cookies();
	}
	
	public void back() {
		browser_back(chptr);
	}

	public void forward() {
		browser_forward(chptr);
	}
	
	public void reload() {
		browser_reload(chptr);
	}
	
	public boolean isLoading() {
		return webpage_loading;
	}
	
	public boolean canGoBack() {
		return can_go_back;
	}
	
	public boolean canGoForward() {
		return can_go_forward;
	}
	
	public static Collection<Chromium> getAllInstances() {
		return chmap.values();
	}
	
	void loadfinish(int bptr) {
		chptr = bptr;
		inited = true;
	}
	
	public void addTitleChangeListener(TitleChangeListener l) {
		tcl = l;
	}
	
	public void addNewWindowListener(NewWindowListener l) {
		nwl = l;
	}
	
	public void addCloseWindowListener(CloseWindowListener l) {
		cwl = l;
	}
	
	public void addLoadStatusListener(LoadStatusListener l) {
		lsl = l;
	}
	
	public void addNavStateListener(NavStateListener l) {
		nsl = l;
	}
	
	void title_change(final String title) {
		if (tcl != null) {
			final Chromium c = this;
			Display display = parentbak.getDisplay();
			if (display.isDisposed()) return;
			display.asyncExec(new Runnable() {
				public void run() {
					tcl.TitleChanged(title, c);
				}
			});
		}
		browser_resized(chptr, hwnd);// force to show the browser
	}
	
	void title_change(String title, int id) {
		chmap.get(id).loaded = true;
		if (title.equals(""))
			return;
		if (chmap.containsKey(id))
			chmap.get(id).title_change(title);
	}
	
	void new_window(final String url) {
		if (nwl != null) {
			//final List<Chromium> c = new ArrayList<Chromium>();
			final Chromium c = this;
			parentbak.getDisplay().syncExec(new Runnable() {
				public void run() {
					nwl.HandleNewWindow(c, url);
				}
			});
			//return c.get(0);
		}
	}
	
	void new_window(int id, String url) {
		if (chmap.containsKey(id))
			chmap.get(id).new_window(url);
	}
	
	void close_window() {
		if (cwl != null) {
			final Chromium c = this;
			parentbak.getDisplay().asyncExec(new Runnable() {
				public void run() {
					cwl.HandleCloseWindow(c);
				}
			});
		}
	}
	
	void close_window(int id) {
		if (chmap.containsKey(id))
			chmap.get(id).close_window();
	}
	
	void load_change(final boolean loading) {
		webpage_loading = loading;
		if (lsl != null) {
			final Chromium c = this;
			parentbak.getDisplay().asyncExec(new Runnable() {
				public void run() {
					lsl.LoadStatusChanged(loading, c);
				}
			});
		}
	}
	
	void load_change(int id, boolean loading) {
		if (chmap.containsKey(id))
			chmap.get(id).load_change(loading);
	}
	
	void navstate_change(boolean canback, boolean canforward) {
		can_go_back = canback;
		can_go_forward = canforward;
		if (nsl != null) {
			final Chromium c = this;
			parentbak.getDisplay().asyncExec(new Runnable() {
				public void run() {
					nsl.NavStateChanged(c);
				}
			});
		}
	}
	
	void navstate_change(int id, boolean canback, boolean canforward) {
		if (chmap.containsKey(id))
			chmap.get(id).navstate_change(canback, canforward);
	}
	
	static {
		System.loadLibrary("libcef");
		System.loadLibrary("chromium_loader_win");
//		System.load("path\\to\\chromium_loader_win.dll");
	}
	
	native void browser_init(int bhwnd, String url, ChromeSettings cset);
	native void browser_new(int bhwnd, int id, String url, ChromeSettings cset);
	native void browser_close(int bptr);
	native void browser_shutdown(int bptr);
	native void browser_clean_cookies();
	
	native void browser_setUrl(int bptr, String url);
	native void browser_resized(int bptr, int bhwnd);
	native void browser_back(int bptr);
	native void browser_forward(int bptr);
	native void browser_reload(int bptr);
}
