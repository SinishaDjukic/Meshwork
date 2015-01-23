package org.meshwork.app.zeroconf.l3.node.gui;

import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.transport.serial.jssc.SerialMessageTransport;
import org.meshwork.core.zeroconf.l3.*;

import java.util.ArrayList;

/**
 * Created by Sinisha Djukic on 2.1.2015.
 */
public class DisconnectDeviceTask extends AbstractDeviceTask {

    public DisconnectDeviceTask(MessageAdapter adapter, AbstractMessageTransport transport) {
        super("Disconnect Device Task", adapter, transport);
    }

    @Override
    public ArrayList<AbstractData> runImpl() throws Throwable {
        doMZCDeinit();
        //We deinit here to avoid hanging the UI... to be fixed
        if ( transport instanceof SerialMessageTransport )
            ((SerialMessageTransport)transport).deinit();
        //TODO update connection state in the GUI?
        return null;
    }
}
