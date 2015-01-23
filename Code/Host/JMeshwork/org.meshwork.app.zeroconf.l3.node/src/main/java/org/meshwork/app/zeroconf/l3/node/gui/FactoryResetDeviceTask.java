package org.meshwork.app.zeroconf.l3.node.gui;

import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.zeroconf.l3.MessageAdapter;

import java.util.ArrayList;

/**
 * Created by Sinisha Djukic on 2.1.2015.
 */
public class FactoryResetDeviceTask extends AbstractDeviceTask {

    public FactoryResetDeviceTask(MessageAdapter adapter, AbstractMessageTransport transport) {
        super("Factory Reset Device Task", adapter, transport);
    }

    @Override
    public ArrayList<AbstractData> runImpl() throws Throwable {
        doMZCFactoryReset();
        //not needed?
        //doMZCDeinit();
        return null;
    }
}
