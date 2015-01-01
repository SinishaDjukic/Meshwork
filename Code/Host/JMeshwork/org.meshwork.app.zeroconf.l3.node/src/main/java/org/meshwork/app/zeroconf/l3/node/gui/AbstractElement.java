package org.meshwork.app.zeroconf.l3.node.gui;

import com.prosyst.pgui.io.IniFile;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public interface AbstractElement {

    public void init(Object context);

    public void deinit(Object context);

    public void setEnabled(boolean enabled);

    public Object getData();

    public void setData(Object data);

    public void read(IniFile ini);

    public void write(IniFile ini);
}
