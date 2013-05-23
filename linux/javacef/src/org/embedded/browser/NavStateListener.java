package org.embedded.browser;

import org.eclipse.swt.internal.SWTEventListener;

public interface NavStateListener extends SWTEventListener {
	void NavStateChanged(Chromium c);
}
