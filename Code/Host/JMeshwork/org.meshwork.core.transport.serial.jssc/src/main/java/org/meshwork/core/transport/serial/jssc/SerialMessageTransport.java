package org.meshwork.core.transport.serial.jssc;

import jssc.*;
import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.MessageData;
import org.meshwork.core.TransportTimeoutException;

import java.io.IOException;

/**
 * Created by Sinisha Djukic on 14-2-12.
 */
public class SerialMessageTransport implements AbstractMessageTransport, SerialPortEventListener {

    protected SerialPort port;
    protected String portName;

    public SerialMessageTransport() {
    }

    public void init(SerialConfiguration config, String portName) throws Exception {
        //todo set up port, etc. and call init(port)
        port = new SerialPort(portName);
        port.openPort();
        port.setParams(config.getBaudRate(), config.getDataBits(), config.getStopBits(), config.getParity(), config.isSetRTS(), config.isSetDTR());
        initPort(port);
    }

    public void deinit() throws Exception {
        port.removeEventListener();
        port.closePort();
        port = null;
        portName = null;
    }

    //port assumed to be set up with correct settings, opened by the caller before calling this method, and having no other event listener added
    public void initPort(SerialPort port) throws Exception {
        if ( port == null )
            throw new IllegalArgumentException("SerialPort cannot be null!");
        this.port = port;
        try {
            port.addEventListener(this);//, SerialPort.MASK_RXCHAR);
        } catch (SerialPortException e) {
            e.printStackTrace();
            this.port = null;
            throw e;
        } finally {
            port.removeEventListener();
        }
        purgeBuffers(true, true);
    }

    //port assumed to be closed by the caller after this method returns
    public void deinitPort() throws Exception {
        if ( port == null )
            throw new IllegalArgumentException("SerialPort not yet initialized!");
        purgeBuffers(true, true);
        try {
            port.removeEventListener();
        } catch (SerialPortException e) {
            e.printStackTrace();
            port = null;
            throw e;
        }
    }

    protected boolean purgeBuffers(boolean read, boolean write) {
        boolean result = false;
        try {
            result = port.purgePort( (read ? (SerialPort.PURGE_RXABORT | SerialPort.PURGE_RXCLEAR) : 0 )
                                   | (write ? (SerialPort.PURGE_TXABORT | SerialPort.PURGE_TXCLEAR) : 0 ) );
        } catch (Exception e) {
            e.printStackTrace();
        }
        return result;
    }

    @Override
    public MessageData readMessage(int timeout) throws TransportTimeoutException, IOException {
        MessageData result = null;
        try {
            result = _readOneMessage(timeout);
        } finally {
            purgeBuffers(true, true);
        }
        return result;
    }

    protected MessageData _readOneMessage(int timeout) throws TransportTimeoutException, IOException {
        MessageData result = new MessageData();
        byte[] header = null;


        try {
            System.out.println("<<<<<<<<<< ENTER SerialMessageTransport._readOneMessage with timeout: "+timeout+" <<<<<<<<<<");
            header = port.readBytes(3, timeout);
        } catch (SerialPortTimeoutException e) {
            throw new TransportTimeoutException("Timeout while reading the message header:" +e.getMessage(), e);
        } catch (Exception e) {
            throw new IOException("Exception while reading the message header: "+e.getMessage(), e);
        }
        result.seq = header[0];
        result.len = header[1];
        result.code = header[2];
        if ( result.len > 1 ) {
            try {
                result.data = port.readBytes(result.len-1, timeout);
            } catch (SerialPortTimeoutException e) {
                throw new TransportTimeoutException("Timeout while reading the message header:" +e.getMessage(), e);
            } catch (Exception e) {
                throw new IOException("Exception while reading the message header: "+e.getMessage(), e);
            }
        }
        System.out.println("<<<<<<<<<< EXIT  SerialMessageTransport._readOneMessage data: "+result+" <<<<<<<<<<");
        return result;
    }

    @Override
    public int sendMessage(MessageData message) throws IOException {
        int result = SEND_NOK;
        try {
            result = _sendOneMessage(message);
        } catch (Exception e) {
            throw new IOException(e);
        } finally {
            purgeBuffers(true, false);
        }
        return result;
    }

    protected int _sendOneMessage(MessageData message) throws SerialPortException {
        System.out.println(">>>>>>>>>> ENTER SerialMessageTransport._sendOneMessage: "+message+" >>>>>>>>>>");
//        port.writeByte(message.seq);
//        port.writeByte(message.len);
//        port.writeByte(message.code);
//        if ( message.len > 1 )
//            port.writeBytes(message.data);
        byte[] temp = new byte[2 + message.len];
        temp[0] = message.seq;
        temp[1] = message.len;
        temp[2] = message.code;
        if ( message.len > 1 )
            System.arraycopy(message.data, 0, temp, 3, message.len-1);
        port.writeBytes(temp);
        System.out.println(">>>>>>>>>> EXIT SerialMessageTransport._sendOneMessage >>>>>>>>>>");
        return SEND_OK;
    }

    @Override
    public void serialEvent(SerialPortEvent serialPortEvent) {
        //TODO decide later on if we should process the data async or not
        //TODO implement start-of-message markers for port disconnection robustness
    }
}
