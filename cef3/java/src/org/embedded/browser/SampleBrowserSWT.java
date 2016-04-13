package org.embedded.browser;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Monitor;
import org.eclipse.swt.widgets.Shell;

/*
 * This can run on all platforms.
 * 
 * This is mainly for Mac and Linux because AWT and SWT can not exist at the
 * same time on Mac. Both Mac and Linux do not support multiple SWT Displays.
 */
public class SampleBrowserSWT {
	public static void main(String[] args) {
		final Display display = new Display();
		Shell shell_control = new Shell(display);
		shell_control.setLayout(new GridLayout(2, true));
		
		GridData gridData = new GridData();
		gridData.horizontalAlignment = GridData.FILL;
        gridData.verticalAlignment = GridData.FILL;
        gridData.grabExcessHorizontalSpace = true;
		
		Button bstart = new Button(shell_control, SWT.NONE);
		bstart.setText("Start");
        bstart.setLayoutData(gridData);
        bstart.addMouseListener(new MouseListener() {
        	public void mouseDoubleClick(MouseEvent e) { }
			public void mouseDown(MouseEvent e) { }
			public void mouseUp(MouseEvent e) {
				ChromeWindow.loadUrl("example.com");
				ChromeWindow.loadUrl("http://127.0.0.1:7000", new ChromeSettings().tabname("javacef", true).right_button(false, true));
				ChromeWindow.loadUrl("http://whatheaders.com/",
						new ChromeSettings().cookie("test", "test").cookie("PREF=ID=1:TM=2:LM=3:S=4; NID=5=6"));
			}
        });
		
		Button bclose = new Button(shell_control, SWT.NONE);
		bclose.setText("Close");
		bclose.setLayoutData(gridData);
		bclose.addMouseListener(new MouseListener() {
        	public void mouseDoubleClick(MouseEvent e) { }
			public void mouseDown(MouseEvent e) { }
			public void mouseUp(MouseEvent e) {
				ChromeWindow.Close(true);
			}
        });
		
		// Load the browser early.
		ChromeWindow.setSharedDisplay(display);
		ChromeWindow.Start();
		
		shell_control.setSize(300, 100);
		shell_control.setText("SampleBrowserSWT");
		
		// Center the control window.
		Monitor primary = display.getPrimaryMonitor();
	    Rectangle bounds = primary.getBounds();
	    Rectangle rect = shell_control.getBounds();
	    int x = bounds.x + (bounds.width - rect.width) / 2;
	    int y = bounds.y + (bounds.height - rect.height) / 2;
	    shell_control.setLocation(x, y);
	    
		shell_control.open();
		
		while (!shell_control.isDisposed()) {
			if (!display.readAndDispatch())
				display.sleep();
		}
		
		ChromeWindow.Shutdown();
	}
}
