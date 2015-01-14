package org.meshwork.core.transport.serial.jssc;

import jssc.*;
import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.MessageData;
import org.meshwork.core.TransportTimeoutException;

import java.io.IOException;
import java.io.PrintWriter;

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
        port.closePort();
        port.removeEventListener();
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
            header = port.readBytes(4, timeout);
        } catch (SerialPortTimeoutException e) {
            throw new TransportTimeoutException("Timeout while reading the message header:" +e.getMessage(), e);
        } catch (Exception e) {
            throw new IOException("Exception while reading the message header: "+e.getMessage(), e);
        }
        result.len = header[0];
        result.seq = header[1];
        result.code = header[2];
        result.subCode = header[3];
        System.out.print("Message header bytes read:\n  ");
        for ( int i = 0; i < 4; i ++ )
            System.out.print(header[i]+" ");
        System.out.println();
        if ( result.len > 1 ) {
            try {
                result.data = port.readBytes(result.len-3, timeout);
                if ( result.data != null ) {
                    System.out.print("Message data bytes read:\n  ");
                    for (int i = 0; i < result.data.length; i++)
                        System.out.print(result.data[i] + " ");
                    System.out.println();
                }
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

    public static final synchronized void printHex(PrintWriter writer, byte[] data,
                                                   int maxCount, int maxPerRow,
                                                   String rowPrefix, String rowSuffix, String separator) {
        int len = data == null ? 0 : data.length;
        int column = 0;
        for ( int i = 0; i < len; i ++ ) {
            if ( i == maxCount ) {
                writer.print("...");
                writer.println(rowSuffix == null ? "" : rowSuffix);
                return;
            }
            writer.print(data[i]);
            if ( i < len - 1 )
                writer.print(separator == null ? ", " : separator);
            if ( ++ column == maxPerRow ) {
                column = 0;
                writer.println(rowSuffix == null ? "" : rowSuffix);
                writer.print(rowPrefix == null ? "" : rowPrefix);
            }
        }
    }
    protected int _sendOneMessage(MessageData message) throws SerialPortException {
        System.out.println(">>>>>>>>>> ENTER SerialMessageTransport._sendOneMessage: "+message+" >>>>>>>>>>");
//        port.writeByte(message.seq);
//        port.writeByte(message.len);
//        port.writeByte(message.subCode);
//        if ( message.len > 1 )
//            port.writeBytes(message.data);
        byte[] temp = new byte[message.len+1];
        temp[0] = message.len;
        temp[1] = message.seq;
        temp[2] = message.code;
        temp[3] = message.subCode;
        int msgdatalen = message.data == null ? 0 : message.data.length;

        System.out.print("Message header bytes to write:\n  ");
        for ( int i = 0; i < 4; i ++ )
            System.out.print(temp[i]+" ");
        System.out.println();

        if ( message.len != 3 + msgdatalen )
            throw new IllegalArgumentException("Message length invalid! message.len ("+message.len+") != 4 + message.data.len("+msgdatalen+")");
        else if ( message.data != null ) {

            System.arraycopy(message.data, 0, temp, 4, message.data.length);

            System.out.print("Message data bytes to write:\n  ");
            for ( int i = 0; i < message.data.length; i ++ )
                System.out.print(message.data[i]+" ");
            System.out.println();
        }

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
