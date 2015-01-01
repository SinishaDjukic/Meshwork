package org.meshwork.app.host.l3.router.console.perf;

import org.meshwork.core.util.Converter;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Properties;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public class TestSendRoutedConfiguration extends AbstractTestConfiguration {

    public static final String CONFIG_KEY_ROUTE_LIST_PREFIX = "perftest.sendrouted.route.";//comma-separated list, multiple keys starting with 0
    public static final String CONFIG_KEY_ITERATION_DELAY   = "perftest.sendrouted.iterationdelay";
    public static final String CONFIG_KEY_MAX_ITERATIONS    = "perftest.sendrouted.maxiterations";
    public static final String CONFIG_KEY_MAX_TIME          = "perftest.sendrouted.maxtime";//seconds

    //list of routes to send to
    //last element of every list is the ultimate destination
    //intermediate hop count must be [0-8]
    //NO VALIDATION DONE HERE!
    public ArrayList<ArrayList<Byte>> routelist;

    public void loadConfiguration(InputStream is) throws Exception {
        Properties p = new Properties();
        p.load(is);
        int i = 0;
        String prop;
        routelist = new ArrayList<ArrayList<Byte>>();
        while ( (prop = p.getProperty(CONFIG_KEY_ROUTE_LIST_PREFIX + i)) != null && !"".equals(prop.trim()) ) {
            routelist.add(Converter.toByteArrayList(CONFIG_KEY_ROUTE_LIST_PREFIX + i, prop));
            i++;
        }
        iterationDelay = Converter.toInt(CONFIG_KEY_ITERATION_DELAY, p.getProperty(CONFIG_KEY_ITERATION_DELAY));
        maxIterations = Converter.toInt(CONFIG_KEY_MAX_ITERATIONS, p.getProperty(CONFIG_KEY_MAX_ITERATIONS));
        maxTime = Converter.toInt(CONFIG_KEY_MAX_TIME, p.getProperty(CONFIG_KEY_MAX_TIME));
    }

}
