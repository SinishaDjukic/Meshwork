package org.meshwork.core.transport.serial.jssc;

import org.meshwork.core.util.Converter;

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

    public void setBaudRate(int baudRate) {
        this.baudRate = baudRate;
    }

    public void setDataBits(int dataBits) {
        this.dataBits = dataBits;
    }

    public void setStopBits(int stopBits) {
        this.stopBits = stopBits;
    }

    public void setParity(int parity) {
        this.parity = parity;
    }

    public void setSetRTS(boolean setRTS) {
        this.setRTS = setRTS;
    }

    public void setSetDTR(boolean setDTR) {
        this.setDTR = setDTR;
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
        baudRate = Converter.toInt(CONFIG_KEY_BAUDRATE, p.getProperty(CONFIG_KEY_BAUDRATE));
        dataBits = Converter.toInt(CONFIG_KEY_DATABITS, p.getProperty(CONFIG_KEY_DATABITS));
        stopBits = Converter.toInt(CONFIG_KEY_STOPBITS, p.getProperty(CONFIG_KEY_STOPBITS));
        parity = Converter.toInt(CONFIG_KEY_PARITY, p.getProperty(CONFIG_KEY_PARITY));
        setRTS = Converter.toBoolean(CONFIG_KEY_SETRTS, p.getProperty(CONFIG_KEY_SETRTS));
        setDTR = Converter.toBoolean(CONFIG_KEY_SETDTR, p.getProperty(CONFIG_KEY_SETDTR));
    }

}
