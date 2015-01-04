package org.meshwork.app.zeroconf.l3.node.gui;

import org.meshwork.core.zeroconf.l3.MZCSerialCfg;
import org.meshwork.core.zeroconf.l3.MZCSerialRes;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public class SerialNumberData extends AbstractData {
    public String value;

    public void read(MZCSerialRes msg) {
        value = (msg.sernumlen == 0 || msg.sernum == null) ? null : new String(msg.sernum);
    }

    public void write(MZCSerialCfg msg) {
        msg.sernum = value == null ? null : value.getBytes();
        msg.sernumlen = (byte) (msg.sernum == null ? 0 : msg.sernum.length);
    }
}
