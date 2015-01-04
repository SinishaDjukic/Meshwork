package org.meshwork.app.zeroconf.l3.node.gui;

import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.zeroconf.l3.*;

import java.util.ArrayList;

/**
 * Created by Sinisha Djukic on 2.1.2015.
 */
public class WriteDeviceTask extends AbstractDeviceTask {

    public WriteDeviceTask(MessageAdapter adapter, AbstractMessageTransport transport) {
        super("Write Device Task", adapter, transport);
    }

    @Override
    public ArrayList<AbstractData> runImpl() throws Throwable {
        ArrayList<AbstractData> input = getInput();

        //unfortunately, these are split in the UI so this requires more work
        DeliveryData deliveryData = null;
        NetworkCapabilitiesData networkCapabilitiesData = null;

        if ( input != null && input.size() > 0 ) {
            doMZCInit();

            for (AbstractData data : input) {
                if ( data instanceof DeliveryData ) {
                    deliveryData = (DeliveryData) data;
                } else if ( data instanceof NetworkCapabilitiesData ) {
                    networkCapabilitiesData = (NetworkCapabilitiesData) data;
                } else if ( data instanceof NetworkConfigurationData ) {
                    MZCNwkCfg mZCNwkCfg = new MZCNwkCfg(nextSeq());
                    ((NetworkConfigurationData)data).write(mZCNwkCfg);
                    doMZCNwkCfg(mZCNwkCfg);
                } else if ( data instanceof ReportingData ) {
                    MZCRepCfg mZCRepCfg = new MZCRepCfg(nextSeq());
                    ((ReportingData)data).write(mZCRepCfg);
                    doMZCRepCfg(mZCRepCfg);
                } else if ( data instanceof SerialNumberData ) {
                    MZCSerialCfg mZCSerialCfg = new MZCSerialCfg(nextSeq());
                    ((SerialNumberData)data).write(mZCSerialCfg);
                    doMZCSerialCfg(mZCSerialCfg);
                }
            }
            //prepare device config
            MZCDevCfg mZCDevCfg = null;
            if ( deliveryData != null ) {
                if ( mZCDevCfg == null )
                    mZCDevCfg = new MZCDevCfg(nextSeq());
                deliveryData.write(mZCDevCfg);
            }
            if ( networkCapabilitiesData != null ) {
                if ( mZCDevCfg == null )
                    mZCDevCfg = new MZCDevCfg(nextSeq());
                networkCapabilitiesData.write(mZCDevCfg);
            }
            //finally, do device config
            if ( mZCDevCfg != null )
                doMZCDevCfg(mZCDevCfg);

            //TODO do we need to deinit automatically or should this be done upon disconnect?
            //doMZCDeinit();

        } else {
            GUILogger.error("[WriteDeviceTask] No input data to write to the device", null);
        }
        return null;
    }
}
