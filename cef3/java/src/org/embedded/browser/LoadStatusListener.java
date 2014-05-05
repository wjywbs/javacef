package org.embedded.browser;

import org.eclipse.swt.internal.SWTEventListener;

public interface LoadStatusListener extends SWTEventListener {
	void LoadStatusChanged(boolean loading, Chromium c);
}
