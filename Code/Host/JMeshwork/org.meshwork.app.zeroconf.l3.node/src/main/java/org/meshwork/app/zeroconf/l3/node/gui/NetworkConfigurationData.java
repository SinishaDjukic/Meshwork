package org.meshwork.app.zeroconf.l3.node.gui;

import org.meshwork.core.zeroconf.l3.MZCNwkCfg;
import org.meshwork.core.zeroconf.l3.MZCNwkRes;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public class NetworkConfigurationData extends AbstractData {

    public byte channelID;
    public short networkID;
    public byte nodeID;
    public String networkKey;

    public void read(MZCNwkRes msg) {
        channelID = msg.channel;
        networkID = msg.nwkid;
        nodeID = msg.nodeid;
        networkKey = (msg.keylen == 0 || msg.key == null) ? null : new String(msg.key);
    }

    public void write(MZCNwkCfg msg) {
        msg.channel = channelID;
        msg.nwkid = networkID;
        msg.nodeid = nodeID;
        msg.key = networkKey == null ? null : networkKey.getBytes();
        msg.keylen = (byte) (msg.key == null ? 0 : msg.key.length);
    }
}
