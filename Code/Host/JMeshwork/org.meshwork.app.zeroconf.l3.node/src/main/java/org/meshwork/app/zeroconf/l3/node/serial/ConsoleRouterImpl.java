package org.meshwork.app.zeroconf.l3.node.serial;

import org.meshwork.app.zeroconf.l3.node.MessageDispatcherImpl;

import org.meshwork.app.zeroconf.l3.node.ZeroConfiguration;
import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.host.l3.MessageAdapter;

import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-12.
 */
public class ConsoleRouterImpl extends AbstractConsole {


    public static void main(String[] args) {
        new ConsoleRouterImpl().init(args);
    }

    public ConsoleRouterImpl() {
    }

    @Override
    protected void printAppHeader() {
        System.out.println("L3 ZeroConf Host App");
        System.out.println("Copyleft 2014, Meshwork Project, Sinisha Djukic");
    }

    @Override
    protected void printStartedMessage() {
        System.out.println("L3 ZeroConf started!");
        System.out.println();
    }

    @Override
    protected MessageDispatcherImpl initMessageDispatcher(MessageAdapter adapter, AbstractMessageTransport transport, ZeroConfiguration routerConfig, PrintWriter writer) throws Exception {
        return new MessageDispatcherImpl(adapter, transport, routerConfig, writer);
    }

}
