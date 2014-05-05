package org.embedded.browser;

import org.eclipse.swt.internal.SWTEventListener;

public interface CloseWindowListener extends SWTEventListener {
	void HandleCloseWindow(Chromium c);
}
