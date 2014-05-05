package org.embedded.browser;

import java.io.File;
import java.util.*;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.custom.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.*;

public class ChromeWindow {
	static Object lock = new Object(), tablock = new Object();
	static ChromeWindow window = null;
	static Display sharedDisplay = null;
	Display display;
	Shell shell;
	CTabFolder folder;
    int count;
    Image[] img;
	boolean complete = false;
	Button back, forward, reload;
	static Display dbg = null;
	static Shell sbg;
	static Chromium cbg;
	
	/**
	 * If you need to increase the performance of loading for the first time,
	 * this function can be called during the initialization of your software
	 * to start the browser engine early. There's no influence not to call 
	 * this function.
	 */
	public static void Start() {
		synchronized (lock) {
			if (dbg == null) {
				if (sharedDisplay == null)
					dbg = new Display();
				else
					dbg = sharedDisplay;
				
				sbg = new Shell(dbg);
				cbg = new Chromium(sbg, SWT.NONE);
				sbg.setAlpha(0);
				sbg.open();
				sbg.setVisible(false);
			}
		}
	}
	
	/**
	 * Set and use the existing display. Multiple displays is not implemented
	 * on Mac and Linux, so this is a workaround.
	 */
	public static void setSharedDisplay(Display display_shared) {
		sharedDisplay = display_shared;
	}
	
	/**
	 * Create a browser instance and navigate to the URL.
	 * @param url Target URL.
	 * @return A new browser instance.
	 */
	public static Chromium loadUrl(String url) {
		return loadUrl(url, new ChromeSettings());
	}
	
	/**
	 * Create a browser instance and navigate to the URL.
	 * @param url Target URL.
	 * @param chromesettings Browser settings.
	 * @return A new browser instance.
	 */
	public static Chromium loadUrl(String url, ChromeSettings chromesettings) {
		Start();
		synchronized (lock) {
			if (window == null || window.shell.isDisposed())
				window = new ChromeWindow();
			
			Chromium c = window.add_browser(url, chromesettings);
			// Need to wait for a short period of time to avoid crash and freeze.
			shortWait();
			return c;
		}
	}
	
	/**
	 * Close all browser tabs.
	 * @param clean_cookies
	 */
	public static void Close(boolean clean_cookies) {
		synchronized (lock) {
			if (window != null)
				if (!window.shell.isDisposed())
					window.display.syncExec(new Runnable() { public void run() { window.shell.close(); } });
			if (clean_cookies && cbg != null)
				cbg.cleanCookies();
		}
	}
	
	/**
	 * Shutdown the browser engine.
	 */
	public static void Shutdown() {
		synchronized (lock) {
			if (dbg != null) {
				sbg.close();
				if (sharedDisplay == null)
					dbg.dispose();
				dbg = null;
			}
		}
	}
	
	static void shortWait() {
		try {
			Thread.sleep(100);
		} catch (InterruptedException ex) {
			ex.printStackTrace();
		}
	}
	
//	Chromium add_browser() {
//		return add_browser("about:blank");
//	}
	
//	Chromium add_browser(String url) {
//		return add_browser(url, new ChromeSettings());
//	}
	
	Chromium add_browser(String url, ChromeSettings chromesettings) {
		return add_browser(folder, url, chromesettings);
	}
	
	Chromium add_browser(CTabFolder folder, String url, ChromeSettings chromesettings) {
		synchronized (tablock) {
			return add_browser_internal(folder, url, chromesettings);
		}
	}
	
	Chromium add_browser_internal(final CTabFolder folder, final String url, final ChromeSettings chromesettings) {
		final ArrayList<Chromium> bl = new ArrayList<Chromium>();
		display.syncExec(new Runnable() {
			public void run() {
				if (folder.isDisposed()) return;
				CTabItem item = new CTabItem(folder, SWT.CLOSE);
				item.setText("New Tab");
				Chromium b = new Chromium(folder, SWT.NONE, url, chromesettings);
				b.setData(item);
				b.setData("pid", -1);
				
				if (chromesettings.tabname != null)
					item.setText(chromesettings.tabname);
				else
					b.addTitleChangeListener(new TitleChangeListener() {
						public void TitleChanged(String title, Chromium c) {
							if (c.isDisposed())
								return;
							CTabItem item = (CTabItem)c.getData();
							if (!item.isDisposed())
								item.setText(title);
						}
					});
				b.addCloseWindowListener(new CloseWindowListener() {
					public void HandleCloseWindow(Chromium c) { ((CTabItem)c.getData()).dispose(); }
				});
				b.addNewWindowListener(new NewWindowListener() {
					public void HandleNewWindow(Chromium c, String url) {
						ChromeSettings csnew = new ChromeSettings(), csprev = c.chromeset;
						if (csprev.rbcascade)
							csnew.right_button(csprev.allow_right_button , csprev.rbcascade);
						if (csprev.tncascade)
							csnew.tabname(csprev.tabname, csprev.tncascade);
						add_browser(url, csnew);
					}
				});
				b.addNavStateListener(new NavStateListener() {
					public void NavStateChanged(Chromium c) { setNavControl(); }
				});
				item.setControl(b);
				item.addDisposeListener(new DisposeListener() {
					public void widgetDisposed(DisposeEvent e) {
						Chromium c = (Chromium)((CTabItem)e.widget).getControl();
						if (c.tab) c.dispose();
						if (folder.getItemCount() == 0)
							folder.getParent().dispose();
					}
				});
				bl.add(b);
				display.timerExec(50, new Runnable() {
					void run_i() {
						if (!folder.isDisposed()) {
							folder.setSelection(folder.getItemCount() - 1);
							setNavControl();
							setFocus();
						}
					}
					public void run() { synchronized (tablock) { run_i(); } }
			    });
			}
		});
		if (bl.size() == 0)
			return null;
		else
			return bl.get(0);
	}
	
	void setFocus() {
		try {
			if (folder.isDisposed() || folder.getItemCount() == 0)
				return;
			Chromium c = (Chromium) folder.getSelection().getControl();
			if (c.isDisposed())
				return;
			c.forceFocus();
			c.browser_resized(c.chptr, c.hwnd);
		}
		catch (Throwable e) { }
	}
	
	void setNavControl() {
		try {
			if (folder.isDisposed() || folder.getItemCount() == 0)
				return;
			Chromium c = (Chromium) folder.getSelection().getControl();
			if (c.isDisposed())
				return;
			back.setEnabled(c.canGoBack());
			forward.setEnabled(c.canGoForward());
		}
		catch (Throwable e) { }
	}
	
	class BrowserControlAdapter extends SelectionAdapter {
		public void widgetSelected(SelectionEvent e) {
			Button b = (Button) e.getSource();
			if (!b.isDisposed() && !folder.isDisposed()) {
				String type = (String) b.getData();
				Chromium c = (Chromium) folder.getSelection().getControl();
				if (type.equals("back"))
					c.back();
				else if (type.equals("forward"))
					c.forward();
				else if (type.equals("reload"))
					c.reload();
			}
		}
	}
	
	ChromeWindow() {
		Thread chromeWindowThread = new Thread(new Runnable() {
			public void run() {
				if (sharedDisplay == null)
					display = new Display();
				else
					display = sharedDisplay;
				
				shell = new Shell(display);
				shell.setLayout(new FillLayout());
				
				folder = new CTabFolder(shell, SWT.BORDER);
				folder.setSimple(false);
				if (SWT.getVersion() < 4200)
					folder.setTabHeight(20);
//				folder.setBackground(new Color(display, 232, 242, 254));
				folder.addMouseListener(new MouseListener() {
					public void mouseDoubleClick(MouseEvent e) { }
					public void mouseDown(MouseEvent e) { setFocus(); setNavControl(); }
					public void mouseUp(MouseEvent e) { setFocus(); setNavControl(); }
				});
				
				String prefix = System.getProperty("user.dir") + File.separator + "resources" + File.separator;
				BrowserControlAdapter bca = new BrowserControlAdapter();
				Composite cb = new Composite(folder, SWT.NONE);
				cb.setLayout(new FillLayout());
				
				back = new Button(cb, SWT.PUSH);
				back.setData("back");
				back.setImage(new Image(display, prefix + "arrow_left.png"));
				back.addSelectionListener(bca);
				
				forward = new Button(cb, SWT.PUSH);
				forward.setData("forward");
				forward.setImage(new Image(display, prefix + "arrow_right.png"));
				forward.addSelectionListener(bca);
				
				reload = new Button(cb, SWT.PUSH);
				reload.setData("reload");
				reload.setImage(new Image(display, prefix + "arrow_refresh.png"));
				reload.addSelectionListener(bca);
				
				folder.setTopRight(cb);
				shell.open();
				
				ImageLoader loader = new ImageLoader();
		        ImageData[] imageDatas = loader.load(prefix + "page-loader.gif");
				count = imageDatas.length;
				img = new Image[count];
		        for (int i = 0; i < count; i++) {
		        	img[i] = new Image(display, imageDatas[i]);
		        }
				display.timerExec(100, new Runnable() {
					public void run() {
						if (!shell.isDisposed()) {
							Collection<Chromium> cl = Chromium.getAllInstances();
							for (Chromium c : cl) {
								if (!c.tab) continue;
								if (c.isDisposed()) continue;
								CTabItem item = (CTabItem)c.getData();
								if (!item.isDisposed()) {
									int pid = Integer.parseInt(c.getData("pid").toString());
									if (c.isLoading()) {
										pid++;
										if (pid == count) pid = 0;
										item.setImage(img[pid]);
									} else {
										if (pid != -1) { pid = -1; item.setImage(null); }
									}
									c.setData("pid", pid);
								}
							}
							display.timerExec(100, this);
						}
					}
				});
				complete = true;
				
				if (sharedDisplay == null) {
					while (!shell.isDisposed())
						if (!display.readAndDispatch())
							display.sleep();
					display.dispose();
				}
				
				synchronized (lock) {
					window = null;
				}
			}
		});
		
		if (sharedDisplay == null)
			chromeWindowThread.start();
		else
			chromeWindowThread.run();
		
		while (!complete)
			try { Thread.sleep(3); } catch (InterruptedException e) { }
	}
}
