package org.meshwork.core.transport.serial.jssc;

import java.io.InputStream;
import java.util.Properties;

/**
 * Created by Sinisha Djukic on 14-2-13.
 */
public class SerialConfiguration {

    //config file property keys
    public static final String CONFIG_KEY_BAUDRATE      = "serial.baudrate";//int
    public static final String CONFIG_KEY_DATABITS      = "serial.databits";//int
    public static final String CONFIG_KEY_STOPBITS      = "serial.stopbits";//int
    public static final String CONFIG_KEY_PARITY        = "serial.parity";//int
    public static final String CONFIG_KEY_SETRTS        = "serial.setrts";//boolean
    public static final String CONFIG_KEY_SETDTR        = "serial.setdtr";//boolean

    protected int baudRate;

    public int getBaudRate() {
        return baudRate;
    }

    public int getDataBits() {
        return dataBits;
    }

    public int getStopBits() {
        return stopBits;
    }

    public int getParity() {
        return parity;
    }

    public boolean isSetRTS() {
        return setRTS;
    }

    public boolean isSetDTR() {
        return setDTR;
    }

    protected int dataBits;
    protected int stopBits;
    protected int parity;
    protected boolean setRTS;
    protected boolean setDTR;

    public SerialConfiguration() {
    }

    public SerialConfiguration(int baudRate, int dataBits, int stopBits, int parity, boolean setRTS, boolean setDTR) {
        this.baudRate = baudRate;
        this.dataBits = dataBits;
        this.stopBits = stopBits;
        this.parity = parity;
        this.setRTS = setRTS;
        this.setDTR = setDTR;
    }

    public void loadConfiguration(InputStream is) throws Exception {
        Properties p = new Properties();
        p.load(is);
        baudRate = toInt(CONFIG_KEY_BAUDRATE, p.getProperty(CONFIG_KEY_BAUDRATE));
        dataBits = toInt(CONFIG_KEY_DATABITS, p.getProperty(CONFIG_KEY_DATABITS));
        stopBits = toInt(CONFIG_KEY_STOPBITS, p.getProperty(CONFIG_KEY_STOPBITS));
        parity = toInt(CONFIG_KEY_PARITY, p.getProperty(CONFIG_KEY_PARITY));
        setRTS = toBoolean(CONFIG_KEY_SETRTS, p.getProperty(CONFIG_KEY_SETRTS));
        setDTR = toBoolean(CONFIG_KEY_SETDTR, p.getProperty(CONFIG_KEY_SETDTR));
    }

    protected boolean toBoolean(String propKey, String propValue) {
        if ( propValue == null )
            throw new IllegalArgumentException("Property '"+propKey+"' is cannot be null!");
        return Boolean.parseBoolean(propValue);
    }

    protected int toInt(String propKey, String propValue) {
        if ( propValue == null )
            throw new IllegalArgumentException("Property '"+propKey+"' is cannot be null!");
        return Integer.parseInt(propValue);
    }

}
