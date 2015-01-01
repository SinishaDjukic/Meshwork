package org.meshwork.app.host.l3.router.console.perf;

import org.meshwork.core.host.l3.Constants;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public class TestSendFloodStats extends TestStats {

    public static final String NAME_SEND_FLOOD = "TestSendFlood";
    public static final int UID_SEND_FLOOD = Constants.DELIVERY_FLOOD;

    public TestSendFloodStats(TestSendFloodConfiguration config) {
        this.config = config;
        testUID = UID_SEND_FLOOD;
    }

    @Override
    public String getTestName() {
        return NAME_SEND_FLOOD;
    }

    @Override
    public String getTestDescription() {
        return "Sends Flood messages to one or more nodes";
    }

    @Override
    public String getTestDetails() {
        StringBuffer sb = new StringBuffer("Node list: ");
        int size = ((TestSendFloodConfiguration)config).dstlist.size();
        for ( int i = 0; i < size; i ++ ) {
            sb.append(((TestSendFloodConfiguration)config).dstlist.get(i));
            if ( i < size - 1 )
                sb.append(", ");
        }
        return sb.toString();
    }
}
