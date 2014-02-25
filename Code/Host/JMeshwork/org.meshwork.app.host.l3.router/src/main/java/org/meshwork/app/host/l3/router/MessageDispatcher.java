package org.meshwork.app.host.l3.router;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.MessageData;
import org.meshwork.core.TransportTimeoutException;
import org.meshwork.core.host.l3.*;

import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by Sinisha Djukic on 14-2-13.
 */
public class MessageDispatcher implements Runnable {

    public static final int MAX_READ_MSG_COUNT_PER_CALL = 100;
    protected AbstractMessageTransport transport;
    protected PrintWriter writer;
    protected MessageAdapter adapter;
    protected RouterConfiguration config;
    protected boolean running;
    protected SimpleDateFormat dateFormatter;
    protected byte seq;
    protected int rfReadTimeout;
    protected int consoleReadTimeout;
    protected RouteMap routeMap;


    public MessageDispatcher(MessageAdapter adapter, AbstractMessageTransport transport,
                             RouterConfiguration config, PrintWriter writer) {
        this.adapter = adapter;
        this.transport = transport;
        this.config = config;
        this.writer = writer;
        rfReadTimeout = config.getRFReadTimeout();
        consoleReadTimeout = config.getConsoleReadTimeout();
        dateFormatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
        routeMap = new RouteMap();
    }

    public RouteMap getRouteMap() {
        return routeMap;
    }

    protected byte nextSeq() {
        return ++seq;
    }

    public void stop() {
        running = false;
    }

    @Override
    public void run() {
        synchronized (this) {
            running = true;
            this.notifyAll();
        }
        AbstractMessage message = null;
        while (running) {
            try {
                writer.print("\n----------- ");
                writer.println(dateFormatter.format(new Date(System.currentTimeMillis())));
                MRFStartReceive startReceive = new MRFStartReceive(nextSeq());
                startReceive.timeout = rfReadTimeout;
                message = sendMessageAndReceive(startReceive);//result = null, NOK, RFRECV, OK_MESSAGE_INTERNAL, OK_MESSAGE_IGNORED
                writer.print("\tReceived Message:\t");
                if ( message != null )
                    message.toString(writer, "\t\t", null, null);
                else
                    writer.print("<Timeout>");
                writer.println();
                if ( message != null ) {
                    switch ( message.getCode() ) {
                        case Constants.MSGCODE_CFGREQUEST: processMCfgRequest(writer, (MConfigRequest) message); break;
                        case Constants.MSGCODE_RFRECV: processMRFReceive(writer, (MRFReceive) message); break;
                    }
                }
                writer.println();
            } catch (Throwable t) {
                writer.println("[MessageDispatcher] Error: " + t.getMessage());
                t.printStackTrace(writer);
                writer.flush();
//                readMessagesAndDiscardAll();
//                t.printStackTrace(writer);
            }
        }
    }

    private void processMRFGetRoute(PrintWriter writer, MRFGetRoute message) throws Exception {
        Route route = null;
        RouteList list = routeMap.getRouteList(message.dst, false);
        if ( list != null ) {
            int count = list.getRouteCount();
            Route r = list.getRoute(message.index);
            if ( r != null )
                route = r;
        }
        AbstractMessage result;
        if ( route == null ) {
            MNOK nok = new MNOK(message.seq);
            nok.error = Constants.ERROR_GENERAL;
            result = nok;
        } else {
            MRFGetRouteRes msg = new MRFGetRouteRes(message.seq);
            msg.route = route;
            result = msg;
        }
        writer.print("\tSending:\t");
        result.toString(writer, "\t\t", null, null);
        writer.println();
        sendMessage(result);
    }

    private void processMRFGetRouteCount(PrintWriter writer, MRFGetRouteCount message) throws Exception {
        MRFGetRouteCountRes msg = new MRFGetRouteCountRes(message.seq);
        RouteList list = routeMap.getRouteList(message.dst, false);
        if ( list != null )
            msg.count = (byte) list.getRouteCount();
        writer.print("\tSending MRFGetRouteCountRes:\t");
        msg.toString(writer, "\t\t", null, null);
        writer.println();
        sendMessage(msg);
    }

    private void processMRFRouteFailed(PrintWriter writer, MRFRouteFailed message) {
        RouteList list = routeMap.getRouteList(message.route.dst, true);
        Route route = list.getRoute(message.route, true);
        message.route.addStatsFailed();
    }

    private void processMRFRouteFound(PrintWriter writer, MRFRouteFound message) {
        RouteList list = routeMap.getRouteList(message.route.dst, true);
        Route route = list.getRoute(message.route, true);
        message.route.addStatsFound();
    }

    private void processMCfgRequest(PrintWriter writer, MConfigRequest message) throws Exception {
        doInitialConfig();
    }

    private void processMRFReceive(PrintWriter writer, MRFReceive message) throws Exception {
        MRFReceiveACK ack = new MRFReceiveACK(message.seq);
        ack.datalen = 0;
        writer.print("\tSending MRFReceiveACK:\t");
        ack.toString(writer, "\t\t", null, null);
        writer.println();
        writer.flush();
        sendMessage(ack);
        boolean finished = false;
        while (!finished) {
            MessageData data = readMessageUntil(consoleReadTimeout, message.seq);
            if ( data != null ) {
                AbstractMessage result = adapter.deserialize(data);
                writer.print("[processMRFReceive] Message received: ");
                result.toString(writer, null, null, null);
                writer.println();
                writer.flush();
                if ( result != null ) {
                    switch ( result.getCode() ) {
                        case Constants.MSGCODE_CFGREQUEST: processMCfgRequest(writer, (MConfigRequest) result); finished = true; break;
                        case Constants.MSGCODE_RFROUTEFOUND: processMRFRouteFound(writer, (MRFRouteFound) result); break;
                        case Constants.MSGCODE_RFROUTEFAILED: processMRFRouteFailed(writer, (MRFRouteFailed) result); break;
                        case Constants.MSGCODE_RFGETROUTECOUNT: processMRFGetRouteCount(writer, (MRFGetRouteCount) result); break;
                        case Constants.MSGCODE_RFGETROUTE: processMRFGetRoute(writer, (MRFGetRoute) result); break;
                        case Constants.MSGCODE_OK: writer.println("... [processMRFReceive] MSGCODE_OK received"); finished = true; break;
                        case Constants.MSGCODE_INTERNAL: writer.println("... [processMRFReceive] MSGCODE_INTERNAL received"); finished = true; break;
                        case Constants.MSGCODE_NOK: writer.println("... [processMRFReceive] MSGCODE_NOK received"); finished = true; break;//throw exception?
                    }
                }
            }
        }
        writer.println("[processMRFReceive] Done");
        writer.flush();
    }

    protected void doInitialConfig() throws Exception {
        doConfigBasic();
        doConfigNetwork();
        doRFInit();
    }

    private void doRFInit() throws Exception {
        MRFInit msg = new MRFInit(nextSeq());
        AbstractMessage result = sendMessageAndReceive(msg);
        System.out.print("[doRFInit] Response: ");
        result.toString(writer, "\t\t", null, null);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending doRFInit");
    }

    private void doRFDenit() throws Exception {
        MRFDeinit msg = new MRFDeinit(nextSeq());
        AbstractMessage result = sendMessageAndReceive(msg);
        System.out.print("[MRFDenit] Response: ");
        result.toString(writer, "\t\t", null, null);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending MRFDenit");
    }

    protected void doConfigBasic() throws Exception {
        MConfigBasic msg = new MConfigBasic(nextSeq());
        msg.nwkcaps = config.getNwkcaps();
        msg.delivery = config.getDelivery();
        msg.retry = config.retry;
        AbstractMessage result = sendMessageAndReceive(msg);
        System.out.print("[doConfigBasic] Response: ");
        result.toString(writer, "\t\t", null, null);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending MConfigBasic");
    }

    protected void doConfigNetwork() throws Exception {
        MConfigNetwork msg = new MConfigNetwork(nextSeq());
        msg.nodeid = config.getNodeid();
        msg.nwkid = config.getNwkid();
        msg.channel = config.getChannel();
        AbstractMessage result = sendMessageAndReceive(msg);
        System.out.print("[doConfigNetwork] Response: ");
        result.toString(writer, "\t\t", null, null);
        if ( result == null || !(result instanceof MOK) )
            throw new Exception("Error sending MConfigNetwork");
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
        System.out.println("[sendMessage] Msg: "+msg);
        msg.toString(writer, "\t\t", null, null);
        writer.println();
        writer.flush();
        MessageData data = new MessageData();
        msg.serialize(data);
        transport.sendMessage(data);
        System.out.println("[sendMessage][DONE] Msg: " + msg);
    }

    protected AbstractMessage sendMessageAndReceive(AbstractMessage msg) throws Exception {
        readMessagesAndDiscardAll();
        sendMessage(msg);
        System.out.println("[sendMessageAndReceive] Receiving with timeout: " + consoleReadTimeout);
        MessageData data = readMessageUntil(consoleReadTimeout, msg.seq);
        System.out.print("[sendMessageAndReceive] Received Data: ");
        if ( data != null )
            data.toString(writer, "\t\t", null, null);
        else
            writer.print("<Null or timeout>");
        writer.println();
        writer.flush();
        AbstractMessage result = data == null ? null : adapter.deserialize(data);
        System.out.print("[sendMessageAndReceive] Received Message: ");
        if ( result != null )
            result.toString(writer, "\t\t", null, null);
        else
            System.out.println("<Null or timeout>");
        writer.println();
        writer.flush();
        return result;
    }

    protected MessageData readMessageUntil(int readTimeout, byte seq) {
        MessageData result = null, temp = null;
        long start = System.currentTimeMillis();
        int count = 0;//prevent endless loop for some unknown reason
        while ( count < MAX_READ_MSG_COUNT_PER_CALL ) {
            try {
                count ++;
                temp = transport.readMessage(readTimeout);
                if ( temp != null ) {
                    if ( temp.seq == seq ) {
                        result = temp;
                        break;
                    } else {
                        AbstractMessage message = adapter.deserialize(temp);
                        if ( message != null && message.getCode() == Constants.MSGCODE_CFGREQUEST ) {
                            processMCfgRequest(writer, (MConfigRequest) message);
                            readMessagesAndDiscardAll();
                            break;
                        } else {
                            System.out.println("<Unexpected message> "+message);
                            if ( message != null )
                                message.toString(writer, null, null, null);
                            writer.println();
                            writer.flush();
                        }
                    }
                }
            } catch (TransportTimeoutException e) {
            } catch (Exception e) {
            }
            //invoked here to ensure at least one read in case readTimeout is really low or zero
            if ( System.currentTimeMillis() - start >= readTimeout )
                break;
        }
        return result;
    }

}
