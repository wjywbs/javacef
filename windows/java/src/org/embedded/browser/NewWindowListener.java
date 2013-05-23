package org.embedded.browser;

import org.eclipse.swt.internal.SWTEventListener;

public interface NewWindowListener extends SWTEventListener {
	void HandleNewWindow(Chromium c, String url);
}
