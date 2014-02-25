package org.meshwork.app.host.l3.router;

import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.host.l3.MessageAdapter;

import java.io.OutputStream;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-12.
 */
public class Router {

    protected PrintWriter writer;
    protected MessageAdapter adapter;
    protected AbstractMessageTransport transport;
    protected RouterConfiguration config;
    protected MessageDispatcher dispatcher;
    protected Thread dispatcherThread;

    public Router() {

    }

    public void init(AbstractMessageTransport transport, RouterConfiguration config, OutputStream out) throws Exception {
        init(transport, config, new PrintWriter(out, true));
    }

    public void init(AbstractMessageTransport transport, RouterConfiguration config, PrintWriter writer) throws Exception {
        if ( transport == null )
            throw new IllegalArgumentException("AbstractMessageTransport cannot be null!");
        if ( config == null )
            throw new IllegalArgumentException("RouterConfiguration cannot be null!");
        if ( writer == null )
            throw new IllegalArgumentException("PrintWriter cannot be null!");
        this.adapter = new MessageAdapter();
        this.transport = transport;
        this.writer = writer;
        this.dispatcher = new MessageDispatcher(adapter, transport, config, writer);
        dispatcher.readMessagesAndDiscardAll();
        dispatcher.doInitialConfig();
        dispatcherThread = new Thread(dispatcher, "[Router] Message Dispatcher Thread");
        //start and wait until the run() method has been invoked
        synchronized (dispatcher) {
            dispatcherThread.start();
            dispatcher.wait();
        }
    }

    public void deinit() throws Exception {
        if ( adapter == null )
            throw new IllegalArgumentException("Router not yet initialized!");
        dispatcher.stop();
        try {
            dispatcherThread.join();
        } finally {
            this.adapter = null;
            this.transport = null;
            this.writer = null;
            dispatcher = null;
            dispatcherThread = null;
        }
    }
}
