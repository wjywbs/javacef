package org.embedded.browser;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.SwingUtilities;

/*
 * An AWT sample implementation of ChromeWindow.
 */
@SuppressWarnings("serial")
public class SampleBrowserAWT extends JFrame {
	JButton bstart, bclose;
	
	public SampleBrowserAWT() {
		bstart = new JButton("Start");
		bstart.setBounds(15, 20, 100, 30);
		bstart.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				// Simple usage.
				ChromeWindow.loadUrl("google.com");
				// Create a tab that all popup tabs have fixed tab name and disabled right click.
				ChromeWindow.loadUrl("http://127.0.0.1:7000", new ChromeSettings().tabname("javacef", true).right_button(false, true));
				// Create a tab with cookies.
				ChromeWindow.loadUrl("http://www.murl.mobi/headers.php", // http://www.ericgiguere.com/tools/http-header-viewer.html
						new ChromeSettings().cookie("test", "test").cookie("PREF=ID=1:TM=2:LM=3:S=4; NID=5=6"));
			}
		});
		this.add(bstart);
		
		bclose = new JButton("Close");
		bclose.setBounds(155, 20, 100, 30);
		bclose.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				// Close all windows and clean cookies.
				// If we do not clean cookies, the new browser will still have
				// the previous cookies.
				ChromeWindow.Close(true);
			}
		});
		this.add(bclose);
		
		this.setTitle("test");
		this.setLayout(null);
		this.setSize(300, 150);
		
		this.addWindowListener(new WindowListener() {
			public void windowOpened(WindowEvent e) {
				// Optional
				ChromeWindow.Start();
			}
			public void windowClosing(WindowEvent e) {
				ChromeWindow.Close(false);
				ChromeWindow.Shutdown();
			}
			public void windowClosed(WindowEvent e) { }
			public void windowIconified(WindowEvent e) { }
			public void windowDeiconified(WindowEvent e) { }
			public void windowActivated(WindowEvent e) { }
			public void windowDeactivated(WindowEvent e) { }
		});
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setLocationRelativeTo(this.getOwner());
		this.setVisible(true);
	}

	public static void main(String[] args) {

		SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				@SuppressWarnings("unused")
				SampleBrowserAWT sampleawt = new SampleBrowserAWT();
			}
		});
	}

}
