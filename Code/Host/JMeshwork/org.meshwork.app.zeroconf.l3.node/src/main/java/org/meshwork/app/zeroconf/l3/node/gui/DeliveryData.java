package org.meshwork.app.zeroconf.l3.node.gui;

import org.meshwork.core.zeroconf.l3.MZCDevCfg;
import org.meshwork.core.zeroconf.l3.MZCDevRes;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public class DeliveryData extends AbstractData {

    public byte delivery;

    public void read(MZCDevRes msg) {
        delivery = msg.delivery;
    }

    public void write(MZCDevCfg msg) {
        msg.delivery = delivery;
    }

}
