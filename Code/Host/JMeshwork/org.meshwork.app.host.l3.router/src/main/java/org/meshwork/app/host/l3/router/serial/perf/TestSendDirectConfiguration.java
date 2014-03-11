package org.meshwork.app.host.l3.router.serial.perf;

import org.meshwork.core.util.Converter;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Properties;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public class TestSendDirectConfiguration extends AbstractTestConfiguration {

    public static final String CONFIG_KEY_DST_LIST          = "perftest.senddirect.dstlist";//comma-separated list
    public static final String CONFIG_KEY_ITERATION_DELAY   = "perftest.senddirect.iterationdelay";//millis
    public static final String CONFIG_KEY_MAX_ITERATIONS    = "perftest.senddirect.maxiterations";
    public static final String CONFIG_KEY_MAX_TIME          = "perftest.senddirect.maxtime";//seconds

    //list of destination nodes to send to
    public ArrayList<Byte> dstlist;

    @Override
    public void loadConfiguration(InputStream is) throws Exception {
        Properties p = new Properties();
        p.load(is);
        dstlist = Converter.toByteArrayList(CONFIG_KEY_DST_LIST, p.getProperty(CONFIG_KEY_DST_LIST));
        iterationDelay = Converter.toInt(CONFIG_KEY_ITERATION_DELAY, p.getProperty(CONFIG_KEY_ITERATION_DELAY));
        maxIterations = Converter.toInt(CONFIG_KEY_MAX_ITERATIONS, p.getProperty(CONFIG_KEY_MAX_ITERATIONS));
        maxTime = Converter.toInt(CONFIG_KEY_MAX_TIME, p.getProperty(CONFIG_KEY_MAX_TIME));
    }


}
