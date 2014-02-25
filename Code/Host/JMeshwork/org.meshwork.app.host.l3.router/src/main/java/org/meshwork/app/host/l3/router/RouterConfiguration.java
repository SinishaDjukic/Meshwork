package org.meshwork.app.host.l3.router;

import java.io.InputStream;
import java.util.Properties;

/**
 * Created by Sinisha Djukic on 14-2-13.
 */
public class RouterConfiguration {

    //config file property keys
    //all values are decimal numbers
    public static final String CONFIG_KEY_NWKCAPS   = "router.nwkcaps";
    public static final String CONFIG_KEY_DELIVERY  = "router.delivery";
    public static final String CONFIG_KEY_RETRY     = "router.retry";
    public static final String CONFIG_KEY_NWKID     = "router.nwkid";
    public static final String CONFIG_KEY_NODEID    = "router.nodeid";
    public static final String CONFIG_KEY_CHANNEL    = "router.channel";
    public static final String CONFIG_KEY_CONSOLE_READ_TIMEOUT    = "router.console.read.timeout";
    public static final String CONFIG_KEY_RF_READ_TIMEOUT    = "router.rf.read.timeout";

    //MConfigBasic
    protected byte nwkcaps;
    protected byte delivery;
    protected byte retry;

    //MConfigNetwork
    protected short nwkid;
    protected byte nodeid;
    protected byte channel;

    //MRFStartReceive
    protected int rfReadTimeout;

    //Serial
    protected int consoleReadTimeout;

    public RouterConfiguration() {
    }

    public RouterConfiguration(byte nwkcaps, byte delivery, byte retry, short nwkid, byte nodeid, int consoleReadTimeout, int rfReadTimeout) {
        this.nwkcaps = nwkcaps;
        this.delivery = delivery;
        this.retry = retry;
        this.nwkid = nwkid;
        this.nodeid = nodeid;
        this.consoleReadTimeout = consoleReadTimeout;
        this.rfReadTimeout = rfReadTimeout;
    }

    public void loadConfiguration(InputStream is) throws Exception {
        Properties p = new Properties();
        p.load(is);
        nwkcaps = toByte(CONFIG_KEY_NWKCAPS, p.getProperty(CONFIG_KEY_NWKCAPS));
        delivery = toByte(CONFIG_KEY_DELIVERY, p.getProperty(CONFIG_KEY_DELIVERY));
        retry = toByte(CONFIG_KEY_RETRY, p.getProperty(CONFIG_KEY_RETRY));
        nwkid = toByte(CONFIG_KEY_NWKID, p.getProperty(CONFIG_KEY_NWKID));
        nodeid = toByte(CONFIG_KEY_NODEID, p.getProperty(CONFIG_KEY_NODEID));
        channel = toByte(CONFIG_KEY_CHANNEL, p.getProperty(CONFIG_KEY_CHANNEL));
        consoleReadTimeout = toInteger(CONFIG_KEY_CONSOLE_READ_TIMEOUT, p.getProperty(CONFIG_KEY_CONSOLE_READ_TIMEOUT));
        rfReadTimeout = toInteger(CONFIG_KEY_RF_READ_TIMEOUT, p.getProperty(CONFIG_KEY_RF_READ_TIMEOUT));
    }

    protected byte toByte(String propKey, String propValue) {
        if ( propValue == null )
            throw new IllegalArgumentException("Property '"+propKey+"' is cannot be null!");
        return Byte.parseByte(propValue);
    }

    protected short toShort(String propKey, String propValue) {
        if ( propValue == null )
            throw new IllegalArgumentException("Property '"+propKey+"' is cannot be null!");
        return Short.parseShort(propValue);
    }

    protected int toInteger(String propKey, String propValue) {
        if ( propValue == null )
            throw new IllegalArgumentException("Property '"+propKey+"' is cannot be null!");
        return Integer.parseInt(propValue);
    }

    public byte getNodeid() {
        return nodeid;
    }

    public byte getNwkcaps() {
        return nwkcaps;
    }

    public byte getDelivery() {
        return delivery;
    }

    public byte getRetry() {
        return retry;
    }

    public short getNwkid() {
        return nwkid;
    }

    public byte getChannel() {
        return channel;
    }

    public int getConsoleReadTimeout() { return consoleReadTimeout; }

    public int getRFReadTimeout() { return rfReadTimeout; }

}
