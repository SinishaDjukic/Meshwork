package org.meshwork.app.host.l3.router.serial;

import org.meshwork.app.host.l3.router.MessageDispatcherImpl;
import org.meshwork.app.host.l3.router.RouterConfiguration;
import org.meshwork.app.host.l3.router.serial.perf.*;
import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.host.l3.MessageAdapter;

import java.io.InputStream;
import java.io.PrintWriter;
import java.util.ArrayList;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public class ConsolePerformanceTestImpl extends AbstractConsole {

    public static void main(String[] args) {
        new ConsolePerformanceTestImpl().init(args);
    }

    public ConsolePerformanceTestImpl() {
    }

    protected void printUsage() {
        System.out.println("Usage: <router config> <serial config> <test config> <serial device>");
        System.out.println("       <router config>=<file name>");
        System.out.println("       <serial config>=<file name>");
        System.out.println("       <test config>=<file name>");
        System.out.println("       <serial device>=<device name/path");
        System.out.println("Example: router.cfg serial.cfg perftest.cfg /dev/ttyUSB0");
        System.out.println("Note that all configs can point to the same file.");
        System.out.println();
    }

    protected void checkParams(String[] args) {
        if ( args == null || args.length == 0 || "-h".equalsIgnoreCase(args[0]) || "-help".equalsIgnoreCase(args[0]) || args.length != 4 ) {
            System.out.println("args.len: "+args.length);
            printUsage();
            exit(EXIT_USAGE, null);
        }
    }

    protected void printAppHeader() {
        System.out.println("L3 Performance Test Host App");
        System.out.println("Copyleft 2014, Meshwork Project, Sinisha Djukic");
    }

    protected void printStartedMessage() {
        System.out.println("L3 Performance Test started!");
        System.out.println();
    }

    @Override
    protected MessageDispatcherImpl initMessageDispatcher(MessageAdapter adapter, AbstractMessageTransport transport, RouterConfiguration routerConfig, PrintWriter writer) throws Exception {
        TestSendDirectConfiguration directConfiguration = new TestSendDirectConfiguration();
        InputStream is = null;
        directConfiguration.loadConfiguration(is = getInputStream(args[2]));
        TestSendDirectStats directStats = new TestSendDirectStats(directConfiguration);
        closeSilently(is);

        TestSendRoutedConfiguration routedConfiguration = new TestSendRoutedConfiguration();
        routedConfiguration.loadConfiguration(is = getInputStream(args[2]));
        TestSendRoutedStats routedStats = new TestSendRoutedStats(routedConfiguration);
        closeSilently(is);

        TestSendFloodConfiguration floodConfiguration = new TestSendFloodConfiguration();
        floodConfiguration.loadConfiguration(is = getInputStream(args[2]));
        TestSendFloodStats floodStats = new TestSendFloodStats(floodConfiguration);
        closeSilently(is);

        ArrayList<TestStats> tests = new ArrayList<TestStats>();
        tests.add(directStats);
        tests.add(routedStats);
        tests.add(floodStats);
        TestSummaryStats summaryStats = new TestSummaryStats(tests);
        return new PefrMessageDispatcherImpl(adapter, transport, routerConfig, writer, summaryStats);
    }

}
