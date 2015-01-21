package org.meshwork.app.zeroconf.l3.node.gui;

import org.meshwork.core.zeroconf.l3.MZCDevCfg;
import org.meshwork.core.zeroconf.l3.MZCDevRes;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public class NetworkCapabilitiesData extends AbstractData {

    public byte capabilities = 0;

    public void read(MZCDevRes msg) {
        capabilities = msg.nwkcaps;
    }

    public void write(MZCDevCfg msg) {
        msg.nwkcaps = capabilities;
    }
}
