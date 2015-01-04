package org.meshwork.app.zeroconf.l3.node.gui;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.MessageData;
import org.meshwork.core.TransportTimeoutException;
import org.meshwork.core.zeroconf.l3.*;

/**
 * Created by Sinisha Djukic on 3.1.2015.
 */
public abstract class AbstractDeviceTask extends AbstractTask {

    public static final int MAX_READ_MSG_COUNT_PER_CALL = 100;

    protected byte seq;
    protected MessageAdapter adapter;
    protected AbstractMessageTransport transport;
    protected int consoleReadTimeout;

    public AbstractDeviceTask(String name, MessageAdapter adapter, AbstractMessageTransport transport) {
        super(name);
        this.adapter = adapter;
        this.transport = transport;
        consoleReadTimeout = 30000;
    }

    protected byte nextSeq() {
        return ++seq;
    }


    protected void readMessagesAndDiscardAll() {
        boolean hasMessages = true;
        int count = 0;//prevent endless loop for some unknown reason
        while ( hasMessages && count < MAX_READ_MSG_COUNT_PER_CALL) {
            try {
                count ++;
                transport.readMessage(10);//low timeout to catch only the buffered messages;
            } catch (TransportTimeoutException e) {
                hasMessages = false;
            } catch (Exception e) {

            }
        }
    }

    protected void sendMessage(AbstractMessage msg) throws Exception {
        GUILogger.info("[sendMessage] Msg: " + msg);
        MessageData data = new MessageData();
        msg.serialize(data);
        transport.sendMessage(data);
        GUILogger.info("[sendMessage][DONE] Msg: " + msg);
    }

    protected AbstractMessage sendMessageAndReceive(AbstractMessage msg) throws Exception {
        GUILogger.info("[sendMessageAndReceive] Entered");
        readMessagesAndDiscardAll();
        sendMessage(msg);
        GUILogger.info("[sendMessageAndReceive] Receiving with timeout: " + consoleReadTimeout);
        MessageData data = readMessageUntil(consoleReadTimeout, msg.seq);
        if ( data != null )
            GUILogger.info("[sendMessageAndReceive] Received Data: "+data);
        else
            GUILogger.error("[sendMessageAndReceive] <Null or timeout>", null);
        AbstractMessage result = data == null ? null : adapter.deserialize(data);
        if ( result != null )
            GUILogger.info("[sendMessageAndReceive] Received Message: "+result);
        else
            GUILogger.error("[sendMessageAndReceive] <Null or timeout>", null);
        return result;
    }

    protected MessageData readMessageUntil(int readTimeout, byte seq) {
        MessageData result = null, temp = null;
        long start = System.currentTimeMillis();
        int count = 0;//prevent endless loop for some unknown reason... oh, sanity
        while (count < MAX_READ_MSG_COUNT_PER_CALL) {
            try {
                count++;
                temp = transport.readMessage(readTimeout);
                boolean breakout = false;
                if (temp != null) {//shouldn't happen?
                    if (temp.seq == seq) {
                        result = temp;
                        breakout = true;
                    } else {
                        AbstractMessage message = adapter.deserialize(temp);
                        GUILogger.error("[readMessageUntil] <Unexpected message> " + message, null);
                    }
                    if ( breakout )
                        break;
                }
            } catch (TransportTimeoutException e) {
                GUILogger.error("*** Transport timeout error: " + e.getMessage(), e);
            } catch (Exception e) {
                GUILogger.error("*** Read error: " + e.getMessage(), e);
            }
            //invoked here to ensure at least one read in case readTimeout is really low or zero
            if (System.currentTimeMillis() - start >= readTimeout)
                break;
        }
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ////////////// L3 ZeroConf Serial Protocol Messages ///////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    /**
     * First session message: initialize the ZeroConf serial protocol sequence.
     *
     * @throws Exception in case of error
     */
    protected void doMZCInit() throws Exception {
        MZCInit msg = new MZCInit(nextSeq());
        AbstractMessage result = sendMessageAndReceive(msg);
        GUILogger.info("[doMZCInit] Response: "+result);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending MZCInit");
    }

    /**
     * Last session message: deinitialize the ZeroConf serial protocol sequence.
     * After this message the device is not required to respond to any other.
     * After this message the device may respond to <code>MZCInit</code>
     * upon its own consent, or may require a special button/reboot sequence
     * before it does.
     *
     * @throws Exception in case of error
     */
    protected void doMZCDeinit() throws Exception {
        MZCDeinit msg = new MZCDeinit(nextSeq());
        AbstractMessage result = sendMessageAndReceive(msg);
        GUILogger.info("[MZCDeinit] Response: "+result);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending MZCDeinit");
    }



    /**
     * Read the current device configuration.
     *
     * @return current device configuration
     * @throws Exception in case of error
     */
    protected MZCDevRes doMZCDevReq() throws Exception {
        MZCDevReq msg = new MZCDevReq(nextSeq());
        AbstractMessage result = sendMessageAndReceive(msg);
        GUILogger.info("[doMZCDevReq] Response: "+result);
        if ( result == null || !(result instanceof MZCDevRes) )
            throw new Exception("Error sending MZCDevReq");
        return (MZCDevRes) result;
    }

    /**
     * Write a device configuration.
     *
     * @param msg device configuration
     * @throws Exception in case of error
     */
    protected void doMZCDevCfg(MZCDevCfg msg) throws Exception {
        GUILogger.info("[doMZCDevCfg] Configuring device: "+msg);
        AbstractMessage result = sendMessageAndReceive(msg);
        GUILogger.info("[doMZCDevCfg] Response: "+result);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending MZCDevCfg");
    }



    /**
     * Read the current network configuration.
     *
     * @return current network configuration
     * @throws Exception in case of error
     */
    protected MZCNwkRes doMZCNwkReq() throws Exception {
        MZCNwkReq msg = new MZCNwkReq(nextSeq());
        AbstractMessage result = sendMessageAndReceive(msg);
        GUILogger.info("[doMZCNwkReq] Response: " + result);
        if ( result == null || !(result instanceof MZCNwkRes) )
            throw new Exception("Error sending MZCNwkReq");
        return (MZCNwkRes) result;
    }

    /**
     * Write a network configuration.
     *
     * @param msg network configuration
     * @throws Exception in case of error
     */
    protected void doMZCNwkCfg(MZCNwkCfg msg) throws Exception {
        GUILogger.info("[doMZCNwkCfg] Configuring network: "+msg);
        AbstractMessage result = sendMessageAndReceive(msg);
        GUILogger.info("[doMZCNwkCfg] Response: "+result);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending MZCNwkCfg");
    }



    /**
     * Read the current reporting configuration.
     *
     * @return current reporting configuration
     * @throws Exception in case of error
     */
    protected MZCRepRes doMZCRepReq() throws Exception {
        MZCRepReq msg = new MZCRepReq(nextSeq());
        AbstractMessage result = sendMessageAndReceive(msg);
        GUILogger.info("[doMZCRepReq] Response: " + result);
        if ( result == null || !(result instanceof MZCRepRes) )
            throw new Exception("Error sending MZCRepReq");
        return (MZCRepRes) result;
    }

    /**
     * Write a reporting configuration.
     *
     * @param msg reporting configuration
     * @throws Exception in case of error
     */
    protected void doMZCRepCfg(MZCRepCfg msg) throws Exception {
        GUILogger.info("[doMZCRepCfg] Configuring reporting: "+msg);
        AbstractMessage result = sendMessageAndReceive(msg);
        GUILogger.info("[doMZRepCfg] Response: "+result);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending MZCRepCfg");
    }



    /**
     * Read the current serial number.
     *
     * @return current serial number
     * @throws Exception in case of error
     */
    protected MZCSerialRes doMZCSerialReq() throws Exception {
        MZCSerialReq msg = new MZCSerialReq(nextSeq());
        AbstractMessage result = sendMessageAndReceive(msg);
        GUILogger.info("[doMZCSerialReq] Response: "+result);
        if ( result == null || !(result instanceof MZCSerialRes) )
            throw new Exception("Error sending MZCSerialReq");
        return (MZCSerialRes) result;
    }

    /**
     * Write a serial number.
     *
     * @param msg new serial number
     * @throws Exception in case of error
     */
    protected void doMZCSerialCfg(MZCSerialCfg msg) throws Exception {
        GUILogger.info("[doMZCSerialCfg] Configuring serial number: "+msg);
        AbstractMessage result = sendMessageAndReceive(msg);
        GUILogger.info("[doMZCSerialCfg] Response: "+result);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending MZCSerialCfg");
    }

}
