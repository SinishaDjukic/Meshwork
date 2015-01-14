package org.meshwork.app.zeroconf.l3.node;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.MessageData;
import org.meshwork.core.TransportTimeoutException;
import org.meshwork.core.zeroconf.l3.*;

import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by Sinisha Djukic on 14-2-13.
 */
public class MessageDispatcherImpl implements MessageDispatcher {

    public static final int MAX_READ_MSG_COUNT_PER_CALL = 100;
    protected AbstractMessageTransport transport;
    protected PrintWriter writer;
    protected MessageAdapter adapter;
    protected ZeroConfiguration config;
    protected boolean running;
    protected SimpleDateFormat dateFormatter;
    protected byte seq;
    protected int consoleReadTimeout;
	protected MZCDevRes lastMZCDevRes;
	protected MZCNwkRes lastMZCNwkRes;

    public MessageDispatcherImpl(MessageAdapter adapter, AbstractMessageTransport transport,
                                 ZeroConfiguration config, PrintWriter writer) {
        this.adapter = adapter;
        this.transport = transport;
        this.config = config;
        this.writer = writer;
        consoleReadTimeout = config.getConsoleReadTimeout();
        dateFormatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
    }

    protected byte nextSeq() {
        return ++seq;
    }

    @Override
    public void run() {
        synchronized (this) {
            running = true;
            this.notifyAll();
        }
        AbstractMessage message = null;
        if (running) {
            try {
                writer.print("\n----------- ");
                writer.println(dateFormatter.format(new Date(System.currentTimeMillis())));
                doMZCInit();
                doMZCDevReq();
                doMZCNwkReq();
                doMZCRepReq();
                doMZCSerialReq();
				if ( config.readonly ) {
					writer.println("[MessageDispatcher] Read-only mode. Will NOT configure the device!");
				} else {
					doMZCCfgNwk();
					doMZCRepCfg();
                    doMZCSerialCfg();
				}
                doMZCDeinit();
                writer.println("[MessageDispatcher] Device configured!");
                writer.print("\n----------- ");
                writer.println();
				if ( lastMZCDevRes != null ) {
					lastMZCDevRes.toString(writer, "\t\t", null, null);
					writer.println();
				}
				if ( lastMZCNwkRes != null ) {
					lastMZCNwkRes.toString(writer, "\t\t", null, null);
					writer.println();
				}
                writer.print("\n----------- ");
            } catch (Throwable t) {
                writer.println("[MessageDispatcher] Error: " + t.getMessage());
                t.printStackTrace(writer);
                writer.flush();
//                readMessagesAndDiscardAll();
//                t.printStackTrace(writer);
            }
        }
        writer.println("[MessageDispatcher] Run complete.");
    }

    @Override
    public void init() throws Exception {
        readMessagesAndDiscardAll();
    }

    @Override
    public void deinit() throws Exception {
        running = false;
		lastMZCDevRes = null;
		lastMZCNwkRes = null;
    }

    protected void doMZCInit() throws Exception {
        MZCInit msg = new MZCInit(nextSeq());
        AbstractMessage result = sendMessageAndReceive(msg);
        writer.print("[doMZCInit] Response: ");
        result.toString(writer, "\t\t", null, null);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending MZCInit");
    }

    protected void doMZCDeinit() throws Exception {
        MZCDeinit msg = new MZCDeinit(nextSeq());
        AbstractMessage result = sendMessageAndReceive(msg);
        writer.print("[MZCDeinit] Response: ");
        if ( result != null )
            result.toString(writer, "\t\t", null, null);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending MZCDeinit");
    }

    protected void doMZCDevReq() throws Exception {
        MZCDevReq msg = new MZCDevReq(nextSeq());
        AbstractMessage result = sendMessageAndReceive(msg);
        writer.print("[doMZCDevReq] Response: ");
        if ( result != null ) {
            result.toString(writer, "\t\t", null, null);
			lastMZCDevRes = (MZCDevRes) result;
		}
        if ( result == null || !(result instanceof MZCDevRes) )
            throw new Exception("Error sending MZCDevReq");
    }

    protected void doMZCNwkReq() throws Exception {
        MZCNwkReq msg = new MZCNwkReq(nextSeq());
        AbstractMessage result = sendMessageAndReceive(msg);
        writer.print("[doMZCNwkReq] Response: ");
        if ( result != null ) {
            result.toString(writer, "\t\t", null, null);
			lastMZCNwkRes = (MZCNwkRes) result;
		}
        if ( result == null || !(result instanceof MZCNwkRes) )
            throw new Exception("Error sending MZCNwkReq");
    }

    protected void doMZCCfgNwk() throws Exception {
        MZCNwkCfg msg = new MZCNwkCfg(nextSeq());
        msg.channel = config.getChannel();
        msg.nodeid = config.getNodeId();
        msg.nwkid = config.getNwkid();
        String k = config.getNwkKey();
        msg.keylen = (byte) (k == null ? 0 : k.length());
        msg.key = msg.keylen == 0 ? null : k.getBytes();
        writer.print("[doMZCCfgNwk] Configuring network: ");
        msg.toString(writer, "\t\t", null, null);
        AbstractMessage result = sendMessageAndReceive(msg);
        writer.print("[doMZCCfgNwk] Response: ");
        if ( result != null )
            result.toString(writer, "\t\t", null, null);
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
        writer.print("[doMZCRepReq] Response: ");
        if ( result == null || !(result instanceof MZCRepRes) )
            throw new Exception("Error sending MZCRepReq");
        return (MZCRepRes) result;
    }

    protected void doMZCRepCfg() throws Exception {
        MZCRepRes msg = new MZCRepRes(nextSeq());
        msg.reportNodeid = config.getReportNodeId();
        msg.reportFlags = config.getReportFlags();
        writer.print("[doMZCRepCfg] Configuring reporting: ");
        msg.toString(writer, "\t\t", null, null);
        AbstractMessage result = sendMessageAndReceive(msg);
        writer.print("[doMZCRepCfg] Response: ");
        if ( result != null )
            result.toString(writer, "\t\t", null, null);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending MZCRepRes");
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
        writer.print("[doMZCSerialReq] Response: " + result);
        if ( result == null || !(result instanceof MZCSerialRes) )
            throw new Exception("Error sending MZCSerialReq");
        return (MZCSerialRes) result;
    }

    protected void doMZCSerialCfg() {
        //TODO implement external serial number configuration
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
        writer.println("[sendMessage] Msg: "+msg);
        msg.toString(writer, "\t\t", null, null);
        writer.println();
        writer.flush();
        MessageData data = new MessageData();
        msg.serialize(data);
        transport.sendMessage(data);
        writer.println("[sendMessage][DONE] Msg: " + msg);
    }

    protected AbstractMessage sendMessageAndReceive(AbstractMessage msg) throws Exception {
        writer.println();
        writer.println("[sendMessageAndReceive] Entered");
        readMessagesAndDiscardAll();
        sendMessage(msg);
        writer.println("[sendMessageAndReceive] Receiving with timeout: " + consoleReadTimeout);
        MessageData data = readMessageUntil(consoleReadTimeout, msg.seq);
        writer.print("[sendMessageAndReceive] Received Data: ");
        if ( data != null )
            data.toString(writer, "\t\t", null, null);
        else
            writer.print("<Null or timeout>");
        writer.println();
        writer.flush();
        AbstractMessage result = data == null ? null : adapter.deserialize(data);
        writer.print("[sendMessageAndReceive] Received Message: ");
        if ( result != null )
            result.toString(writer, "\t\t", null, null);
        else
            writer.println("<Null or timeout>");
        writer.println();
        writer.println("[sendMessageAndReceive] Exited");
        writer.println();
        writer.flush();
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
                        writer.println("<Unexpected message> " + message);
                        if (message != null)
                            message.toString(writer, null, null, null);
                        writer.println();
                        writer.flush();
                    }
                    if ( breakout )
                        break;
                }
            } catch (TransportTimeoutException e) {
                writer.println("*** Transport timeout error: "+e.getMessage());
                writer.flush();
            } catch (Exception e) {
                writer.println("*** Read error: "+e.getMessage());
                e.printStackTrace(writer);
                writer.flush();
            }
            //invoked here to ensure at least one read in case readTimeout is really low or zero
            if (System.currentTimeMillis() - start >= readTimeout)
                break;
        }
        return result;
    }

}
