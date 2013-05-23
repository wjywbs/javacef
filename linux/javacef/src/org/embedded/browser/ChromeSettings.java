package org.embedded.browser;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

public class ChromeSettings {
	String tabname;
	boolean tncascade;
	
	boolean allow_right_button, rbcascade;
	
	HashMap<String, String> cookies;
	String[] keys, values;
	
	public ChromeSettings() {
		tabname = null;
		tncascade = false;
		allow_right_button = true;
		rbcascade = false;
		cookies = new HashMap<String, String>();
	}
	
	/**
	 * Set tab name or use default web title.
	 * @param name If null is passed in, the tab name will return to default.
	 * @param cascade Indicates whether the tab name stays the same when a new tab opens from this tab.
	 * @return
	 */
	public ChromeSettings tabname(String name, boolean cascade) {
		this.tabname = name;
		this.tncascade = cascade;
		return this;
	}
	
	public ChromeSettings right_button(boolean allow_right_button, boolean cascade) {
		this.allow_right_button = allow_right_button;
		this.rbcascade = cascade;
		return this;
	}
	
	/**
	 * Set or delete a cookie.
	 * @param key
	 * @param value If null is passed in, the cookie will be removed.
	 * @return
	 */
	public ChromeSettings cookie(String key, String value) {
		if (value == null) {
			if (cookies.containsKey(key))
				cookies.remove(key);
		}
		else
			cookies.put(key, value);
		return this;
	}
	
	/**
	 * Set cookies from cookies string.
	 * @param cookie_string
	 * @return
	 */
	public ChromeSettings cookie(String cookie_string) {
		String[] pairs = cookie_string.split("=(?=;)|; ?");
		for (int i = 0; i < pairs.length; i++) {
			String[] kv = pairs[i].split("=", 2);
			if (kv.length == 2)
				cookie(kv[0], kv[1]);
		}
		return this;
	}
	
	ChromeSettings Finalize() {
		keys = new String[cookies.size()];
		values = new String[cookies.size()];
		int i = 0;
		Iterator<Entry<String, String>> ci = cookies.entrySet().iterator();
		Entry<String, String> c;
		while (ci.hasNext()) {
			c = ci.next();
			keys[i] = c.getKey();
			values[i] = c.getValue();
			i++;
		}
		return this;
	}
}
