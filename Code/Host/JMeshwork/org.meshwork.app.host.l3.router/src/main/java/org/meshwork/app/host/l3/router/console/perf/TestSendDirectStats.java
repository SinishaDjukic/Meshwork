package org.meshwork.app.host.l3.router.console.perf;

import org.meshwork.core.host.l3.Constants;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public class TestSendDirectStats extends TestStats {

    public static final String NAME_SEND_DIRECT = "TestSendDirect";
    public static final int UID_SEND_DIRECT = Constants.DELIVERY_DIRECT;

    public TestSendDirectStats(TestSendDirectConfiguration config) {
        this.config = config;
        testUID = UID_SEND_DIRECT;
    }

    @Override
    public String getTestName() {
        return NAME_SEND_DIRECT;
    }

    @Override
    public String getTestDescription() {
        return "Sends Direct messages to one or more nodes";
    }

    @Override
    public String getTestDetails() {
        StringBuffer sb = new StringBuffer("Node list: ");
        int size = ((TestSendDirectConfiguration)config).dstlist.size();
        for ( int i = 0; i < size; i ++ ) {
            sb.append(((TestSendDirectConfiguration)config).dstlist.get(i));
            if ( i < size - 1 )
                sb.append(", ");
        }
        return sb.toString();
    }
}
