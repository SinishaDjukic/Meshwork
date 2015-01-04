package org.meshwork.app.zeroconf.l3.node.gui;

import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.zeroconf.l3.*;

import java.util.ArrayList;

/**
 * Created by Sinisha Djukic on 2.1.2015.
 */
public class ReadDeviceTask extends AbstractDeviceTask {

    public ReadDeviceTask(MessageAdapter adapter, AbstractMessageTransport transport) {
        super("Read Device Task", adapter, transport);
    }

    @Override
    public ArrayList<AbstractData> runImpl() throws Throwable {
        doMZCInit();

        MZCDevRes mZCDevRes = doMZCDevReq();
        MZCNwkRes mZCNwkRes = doMZCNwkReq();
        MZCRepRes mZCRepRes = doMZCRepReq();
        MZCSerialRes mZCSerialRes = doMZCSerialReq();

        //TODO do we need to deinit automatically or should this be done upon disconnect?
        //doMZCDeinit();

        ArrayList<AbstractData> result = new ArrayList<AbstractData>(10);

        DeliveryData deliveryData = new DeliveryData();
        deliveryData.read(mZCDevRes);
        NetworkCapabilitiesData networkCapabilitiesData = new NetworkCapabilitiesData();
        networkCapabilitiesData.read(mZCDevRes);
        NetworkConfigurationData networkConfigurationData = new NetworkConfigurationData();
        networkConfigurationData.read(mZCNwkRes);
        ReportingData reportingData = new ReportingData();
        reportingData.read(mZCRepRes);
        SerialNumberData serialNumberData = new SerialNumberData();
        serialNumberData.read(mZCSerialRes);

        result.add(deliveryData);
        result.add(networkCapabilitiesData);
        result.add(networkConfigurationData);
        result.add(reportingData);
        result.add(serialNumberData);

        return result;
    }
}
