package org.embedded.browser;

import java.text.DecimalFormat;

import org.eclipse.swt.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

public class DownloadWindow {
	static String init_path = javax.swing.filechooser.FileSystemView
			.getFileSystemView().getDefaultDirectory().getAbsolutePath();
	static Display display = new Display();
	final int unit = 1024;
	final int S_DOWNLOADING = 1;
	final int S_DOWNLOADED = 2;
	final int S_ERROR = 3;
	int dh, bid, status;
	String filename, mime_type, sz;
	long size, received;
	volatile boolean loaded = false;
	Shell shell;
	ProgressBar bar;
	Label lbl;

	public DownloadWindow(String fn, long fsize, String mime) {
		shell = new Shell(display);
		filename = fn;
		size = fsize;
		mime_type = mime;
		sz = setSize(size);
		received = 0;
	}

	String setSize(long size) {
		String sz;
		if (size < unit)
			sz = size + " B";
		else {
			long kb = size / unit;
			if (kb < unit)
				sz = kb + " KB";
			else {
				DecimalFormat df = new DecimalFormat();
				df.setMinimumFractionDigits(0);
				df.setMaximumFractionDigits(1);
				double mb = (double) size / unit / unit;
				sz = df.format(mb) + " MB";
			}
		}
		return sz;
	}

	String getPathAndInit() {
		final FileDialog dialog = new FileDialog(shell, SWT.SAVE);
		dialog.setFilterPath(init_path);
		dialog.setText(filename + ", " + sz + ", " + mime_type);
		dialog.setFileName(filename);
		String path = dialog.open();
		shell.dispose();
		if (path == null)
			path = "";
		else {
			new Thread() {
				public void run() {
					final Display display = new Display();
					shell = new Shell(display, SWT.CLOSE | SWT.MIN);
					FillLayout fl = new FillLayout();
					fl.type = SWT.VERTICAL;
					fl.marginHeight = 8;
					fl.marginWidth = 8;
					shell.setLayout(fl);
					lbl = new Label(shell, SWT.NONE);
					lbl.setText(dialog.getFileName() + ", " + sz + ", " + mime_type);
					if (size > 0)
						bar = new ProgressBar(shell, SWT.SMOOTH);
					else
						bar = new ProgressBar(shell, SWT.SMOOTH | SWT.INDETERMINATE);
					bar.setSelection(0);
					shell.pack();
					shell.open();
					new Thread() {
						public void run() {
							while (true) {
								try {
									Thread.sleep(1000);
								} catch (InterruptedException e) { }
								if (!loaded) continue;
								if (status == S_DOWNLOADING || status == S_DOWNLOADED) {
									if (display.isDisposed() || bar.isDisposed() || shell.isDisposed())
										return;
									final long r = getReceived(dh), delta = r - received;
									received = r;
									display.syncExec(new Runnable() {
										public void run() {
											if (bar.isDisposed() || lbl.isDisposed())
												return;
											if (r > 0) {
												double d = ((double) r) / size * 100;
												if (size < 0) { d = 0; }
												int percent = (int) d;
												shell.setText(percent + "%, " + setSize(r) + ", " + setSize(delta) + "/s");
												bar.setSelection(percent);
											}
										}
									});
									if (r == -10) { status = S_ERROR; } //defined in c++
								}
								if (status == S_DOWNLOADED || status == S_ERROR) {
									display.syncExec(new Runnable() {
										public void run() {
											if (status == S_ERROR) {
												MessageBox err = new MessageBox(shell, SWT.OK | SWT.ICON_ERROR);
												err.setText("����ʧ��");
												err.setMessage("�ļ�" + dialog.getFileName() + "����ʧ��");
												err.open();
											}
											if (!shell.isDisposed())
												shell.dispose();
										}
									});
									break;
								}
							}
						}
					}.start();
					while (!shell.isDisposed()) {
						if (!display.readAndDispatch())
							display.sleep();
					}
					display.dispose();
				}
			}.start();
		}
		return path;
	}

	void set_dhptr(int dhp) {
		dh = dhp;
		status = S_DOWNLOADING;
		loaded = true;
	}

	void set_status(int s) {
		status = s;
	}

	long getReceived(int dh) {
		if (status == S_DOWNLOADING)
			return getReceivedN(dh);
		else
			return size;//avoid invalid memory access
	}

	static {
		System.loadLibrary("chromium_loader");
	}

	native long getReceivedN(int dh);
}
