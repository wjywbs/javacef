package org.embedded.browser;

import org.eclipse.swt.SWT;
import org.eclipse.swt.browser.Browser;
import org.eclipse.swt.custom.CTabFolder;
import org.eclipse.swt.custom.CTabItem;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

public class SampleWindow {

	public static void main(String[] args) {
//		single_browser();
		test_chromewindow();
//		test_ctabfolder();
	}

	static void single_browser() {
		final Display display = new Display();
		final Shell shell = new Shell(display);
		shell.setLayout(new FillLayout());

		/*
		 * // Method 1 Chromium.browser_init(shell.vboxHandle,
		 * "http://google.com", new ChromeSettings().Finalize());
		 * display.timerExec(10, new Runnable() { public void run() {
		 * Chromium.browser_message_loop(); display.timerExec(10, this); }});
		 * shell.addControlListener(new ControlListener() { public void
		 * controlMoved(ControlEvent arg0) { } public void
		 * controlResized(ControlEvent arg0) {
		 * Chromium.browser_resized(shell.vboxHandle, shell.vboxHandle); } });//
		 */

		Chromium c = new Chromium(shell, SWT.NONE, "google.com");
		shell.open();

		// The trim size makes the size much larger. Don't do this.
		// c.setSize(shell.getSize());
		while (!shell.isDisposed()) {
			if (!display.readAndDispatch())
				display.sleep();
		}
	}
	
	static void test_chromewindow() {
		 ChromeWindow.loadUrl("google.com");
		 ChromeWindow.loadUrl("google.com");
		 ChromeWindow.loadUrl("google.com");
	}
	
	static void setFocus(CTabFolder folder) {
		try {
			Text c = (Text) folder.getSelection().getControl();
			c.forceFocus();
			//c.browser_resized(c.chptr, c.hwnd); // Windows only.
		}
		catch (Throwable e) { }
	}

	static void test_ctabfolder() {
		final Display display = new Display();
		final Shell shell = new Shell(display);
		shell.setLayout(new FillLayout());

		final CTabFolder folder = new CTabFolder(shell, SWT.BORDER);
		folder.setSimple(false);
		for (int i = 0; i < 10; i++) {
			CTabItem item = new CTabItem(folder, SWT.CLOSE);
			item.setText("New Tab");
			Text b = new Text(folder, SWT.NONE);
			item.setControl(b);
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
	}
}
