package org.embedded.browser;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.CTabFolder;
import org.eclipse.swt.custom.CTabItem;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

public class SampleWindow {

	public static void main(String[] args) {
		single_browser();
//		test_chromewindow_windows_only();
//		test_chromewindow();
//		test_ctabfolder();
	}

	static void single_browser() {
		final Display display = new Display();
		final Shell shell = new Shell(display);
		shell.setLayout(new FillLayout());

		/**
		 * single_browser() works without the following fix on Linux,
		 * but emits errors when the window is closed. Create a hidden shell
		 * with a blank browser first and close this hidden shell on exit, so
		 * gtk_delete_event is not sent to the hidden shell to avoid interfering
		 * browser_shutdown of the first browser.
		 */
		if (System.getProperty("os.name").equals("Linux"))
			linux_fix_before(display);

		Chromium c = new Chromium(shell, SWT.NONE, "google.com");
		c.addTitleChangeListener(new TitleChangeListener() {
			public void TitleChanged(String title, Chromium browser) {
				shell.setText(title);
			}
		});
		shell.open();

		while (!shell.isDisposed()) {
			if (!display.readAndDispatch())
				display.sleep();
		}

		if (System.getProperty("os.name").equals("Linux"))
			linux_fix_after(display);
	}
	
	static void linux_fix_before(Display display) {
		Shell shell_background = new Shell(display);
		new Chromium(shell_background, SWT.NONE);
		shell_background.setAlpha(0);
		shell_background.open();
		shell_background.setVisible(false);
		display.setData("linux_fix_shell", shell_background);
	}
	
	static void linux_fix_after(Display display) {
		Shell shell_background = (Shell) display.getData("linux_fix_shell");
		if (shell_background != null) {
			shell_background.close();
		}
	}
	
	/**
	 * Not available on Mac and Linux: Use test_chromewindow() or
	 * SampleBrowserSWT instead.
	 */
	static void test_chromewindow_windows_only() {
		ChromeWindow.loadUrl("google.com");
		ChromeWindow.loadUrl("google.com");
		ChromeWindow.loadUrl("google.com");
	}
	
	/**
	 * This function supports all platforms, but it uses private fields
	 * (ChromeWindow.window).
	 */
	static void test_chromewindow() {
		Display display = new Display();
		ChromeWindow.setSharedDisplay(display);
		
		ChromeWindow.loadUrl("google.com");
		ChromeWindow.loadUrl("google.com");
		ChromeWindow.loadUrl("google.com");
		
		while (!ChromeWindow.window.shell.isDisposed())
			if (!display.readAndDispatch())
				display.sleep();
		
		ChromeWindow.Shutdown();
	}
	
	static void setFocus(CTabFolder folder) {
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

	static void test_ctabfolder() {
		final Display display = new Display();
		final Shell shell = new Shell(display);
		shell.setLayout(new FillLayout());

		if (System.getProperty("os.name").equals("Linux"))
			linux_fix_before(display);

		final CTabFolder folder = new CTabFolder(shell, SWT.BORDER);
		folder.setSimple(false);
		for (int i = 0; i < 5; i++) {
			CTabItem item = new CTabItem(folder, SWT.CLOSE);
			item.setText("New Tab");
			//Text b = new Text(folder, SWT.NONE);
			Chromium b = new Chromium(folder, SWT.NONE, "example.com");
			item.setControl(b);
			item.addDisposeListener(new DisposeListener() {
				public void widgetDisposed(DisposeEvent e) {
					Chromium c = (Chromium)((CTabItem)e.widget).getControl();
					if (c.tab)
						c.dispose();
					if (folder.getItemCount() == 0)
						folder.getParent().dispose();
				}
			});
		}
		folder.addMouseListener(new MouseListener() {
			public void mouseDoubleClick(MouseEvent e) { }
			public void mouseDown(MouseEvent e) { setFocus(folder); }
			public void mouseUp(MouseEvent e) { setFocus(folder); }
		});

		shell.open();
		while (!shell.isDisposed()) {
			if (!display.readAndDispatch())
				display.sleep();
		}

		if (System.getProperty("os.name").equals("Linux"))
			linux_fix_after(display);
	}
}
