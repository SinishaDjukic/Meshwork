package org.meshwork.app.host.l3.router.serial.perf;

import org.meshwork.core.host.l3.Constants;

import java.util.ArrayList;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public class TestSendRoutedStats extends TestStats {

    public static final String NAME_SEND_ROUTED = "TestSendRouted";
    public static final int UID_SEND_ROUTED = Constants.DELIVERY_ROUTED;

    public TestSendRoutedStats(TestSendRoutedConfiguration config) {
        this.config = config;
        testUID = UID_SEND_ROUTED;
    }

    @Override
    public String getTestName() {
        return NAME_SEND_ROUTED;
    }

    @Override
    public String getTestDescription() {
        return "Sends Routed messages to one or more nodes";
    }

    @Override
    public String getTestDetails() {
        int routecount = ((TestSendRoutedConfiguration)config).routelist.size();
        StringBuffer sb = new StringBuffer("Route count: ").append(routecount);
        if ( routecount > 0 ) {
            for ( int i = 0; i < routecount; i ++ ) {
                sb.append("\r\n\tRoute ").append(i).append(": ");
                    ArrayList<Byte> route = (ArrayList<Byte>) ((TestSendRoutedConfiguration)config).routelist.get(i);
                    int size = route.size() - 1;
                    sb.append(route.get(i));
                    if ( i < size - 1 )
                        sb.append(", ");
            }
        }
        return sb.toString();
    }
}
