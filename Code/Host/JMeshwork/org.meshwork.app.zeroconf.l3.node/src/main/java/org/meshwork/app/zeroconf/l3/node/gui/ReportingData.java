package org.meshwork.app.zeroconf.l3.node.gui;

import org.meshwork.core.zeroconf.l3.MZCRepCfg;
import org.meshwork.core.zeroconf.l3.MZCRepRes;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public class ReportingData extends AbstractData {

    public byte reportingFlags = 0;
    public byte reportingNodeID = 0;

    public void read(MZCRepRes msg) {
        reportingFlags = msg.reportFlags;
        reportingNodeID = msg.reportNodeid;
    }

    public void write(MZCRepCfg msg) {
        msg.reportFlags = reportingFlags;
        msg.reportNodeid = reportingNodeID;
    }
}
