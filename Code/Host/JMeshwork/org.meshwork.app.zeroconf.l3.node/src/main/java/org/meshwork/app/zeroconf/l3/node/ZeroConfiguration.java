package org.meshwork.app.zeroconf.l3.node;

import org.meshwork.core.util.Converter;

import java.io.InputStream;
import java.util.Properties;

/**
 * Created by Sinisha Djukic on 14-2-13.
 */
public class ZeroConfiguration {

    //config file property keys
    //all values are decimal numbers
    public static final String CONFIG_KEY_NWKKEY                = "zeroconf.nwkkey";
    public static final String CONFIG_KEY_NWKID                 = "zeroconf.nwkid";
    public static final String CONFIG_KEY_NODEID                = "zeroconf.nodeid";
    public static final String CONFIG_KEY_CHANNEL               = "zeroconf.channel";
    public static final String CONFIG_KEY_CONSOLE_READ_TIMEOUT  = "zeroconf.console.read.timeout";
    public static final String CONFIG_KEY_REPORT_NODEID         = "zeroconf.report.reportNodeid";
    public static final String CONFIG_KEY_REPORT_FLAGS          = "zeroconf.report.flags";
    public static final String CONFIG_KEY_READONLY          	= "zeroconf.readonly";

    //MZCCfgNetwork
    protected byte channel;
    protected short nwkid;
    protected byte nodeid;
    protected String nwkkey;
	
	//MZCCfgRep
    protected byte reportNodeid;
    protected byte reportFlags;

	//Only read the device configuration, don't change
    protected boolean readonly;

    //Serial
    protected int consoleReadTimeout;

    public ZeroConfiguration() {
    }

    public ZeroConfiguration(byte channel, short nwkid, byte nodeid, String networkkey, int consoleReadTimeout) {
        this.channel = channel;
        this.nwkid = nwkid;
        this.nodeid = nodeid;
        this.nwkkey = networkkey;
        this.consoleReadTimeout = consoleReadTimeout;
    }

    public void loadConfiguration(InputStream is) throws Exception {
        Properties p = new Properties();
        p.load(is);
        channel = Converter.toByte(CONFIG_KEY_CHANNEL, p.getProperty(CONFIG_KEY_CHANNEL));
        nwkid = Converter.toByte(CONFIG_KEY_NWKID, p.getProperty(CONFIG_KEY_NWKID));
        nodeid = Converter.toByte(CONFIG_KEY_NODEID, p.getProperty(CONFIG_KEY_NODEID));
        nwkkey = p.getProperty(CONFIG_KEY_NWKKEY);
        reportNodeid = Converter.toByte(CONFIG_KEY_REPORT_NODEID, p.getProperty(CONFIG_KEY_REPORT_NODEID));
        reportFlags = Converter.toByte(CONFIG_KEY_REPORT_FLAGS, p.getProperty(CONFIG_KEY_REPORT_FLAGS));
        consoleReadTimeout = Converter.toInt(CONFIG_KEY_CONSOLE_READ_TIMEOUT, p.getProperty(CONFIG_KEY_CONSOLE_READ_TIMEOUT));
        readonly = Converter.toBoolean(CONFIG_KEY_READONLY, p.getProperty(CONFIG_KEY_READONLY));
    }

    public byte getChannel() {
        return channel;
    }

    public short getNwkid() {
        return nwkid;
    }

    public byte getNodeid() {
        return nodeid;
    }

    public String getNwkkey() {
        return nwkkey;
    }

    public int getConsoleReadTimeout() { return consoleReadTimeout; }

    public void setNwkid(short nwkid) {
        this.nwkid = nwkid;
    }

    public void setNodeid(byte nodeid) {
        this.nodeid = nodeid;
    }

    public void setChannel(byte channel) {
        this.channel = channel;
    }

    public void setNwkkey(String nwkkey) {
        this.nwkkey = nwkkey;
    }

    public void setConsoleReadTimeout(int consoleReadTimeout) {
        this.consoleReadTimeout = consoleReadTimeout;
    }

    public byte getReportNodeid() {
        return reportNodeid;
    }

    public void setReportNodeid(byte reportNodeid) {
        this.reportNodeid = reportNodeid;
    }

    public byte getReportFlags() {
        return reportFlags;
    }

    public void setReportFlags(byte reportFlags) {
        this.reportFlags = reportFlags;
    }

    public boolean getReadonly() {
        return readonly;
    }

    public void setReadonly(boolean readonly) {
        this.readonly = readonly;
    }

}
