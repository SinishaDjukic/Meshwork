package org.meshwork.app.host.l3.router.console;

import org.meshwork.app.host.l3.router.MessageDispatcherImpl;
import org.meshwork.app.host.l3.router.RouterConfiguration;
import org.meshwork.app.host.l3.router.console.perf.*;
import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.host.l3.MessageAdapter;
import org.meshwork.core.transport.serial.jssc.SerialConfiguration;

import java.io.*;
import java.util.ArrayList;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public class ConsolePerformanceTestArrayImpl extends AbstractConsole {

    public static void main(String[] args) {
        new ConsolePerformanceTestArrayImpl().init(args);
    }

    public ConsolePerformanceTestArrayImpl() {
    }

    protected void printUsage() {
        System.out.println("Usage: <router config> <serial config> <test config> <serial device list>");
        System.out.println("       <router config>=<file name>");
        System.out.println("       <serial config>=<file name>");
        System.out.println("       <test config>=<file name>");
        System.out.println("       <serial device list>=<device>[ <device>]");
        System.out.println("Example: router.cfg serial.cfg perftest.cfg /dev/ttyUSB0;/dev/ttyUSB1;/dev/ttyUSB2");
        System.out.println("Note that all configs can point to the same file.");
        System.out.println();
    }

    protected void checkParams(String[] args) {
        if ( args == null || args.length == 0 || "-h".equalsIgnoreCase(args[0]) || "-help".equalsIgnoreCase(args[0]) || args.length < 4 ) {
            System.out.println("Invalid param count: "+args.length);
            printUsage();
            exit(EXIT_USAGE, null);
        }
    }

    protected void printAppHeader() {
        System.out.println("L3 Performance Test Array Host App");
        System.out.println("Copyleft 2014, Meshwork Project, Sinisha Djukic");
    }

    protected void printStartedMessage() {
        System.out.println("L3 Performance Test Array started!");
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
        PerfMessageDispatcherImpl result = new PerfMessageDispatcherImpl(adapter, transport, routerConfig, writer, summaryStats);

        byte nodes = 0;
        int testNodeCount = args.length - 4;//last param is the test router node, so exclude it
        //check if testNodeCount < 254, since we need 1 node for the base, and 0 and 255 are reserved
        if ( testNodeCount > 253 )
            throw new IllegalArgumentException("Number of test nodes '"+testNodeCount+"' exceeds 253!");
        String dev = null;
        File outBaseDir = new File("./logs/");
        outBaseDir.mkdirs();

        byte baseNodeId = routerConfig.getNodeId();

        writer.println("Initializing base and '"+testNodeCount+"' devices...");
        for ( int i = 0; i < testNodeCount; i ++ ) {
            try {
                dev = args[3 + i];

                ConsoleRouterImpl c = new ConsoleRouterImpl();
                RouterConfiguration rc = c.initRouterConfiguration(args[0].trim());
                SerialConfiguration sc = c.initSerialConfiguration(args[1].trim());
                AbstractMessageTransport tr = c.initTransport(sc, dev);

                byte nodeId = (byte) (baseNodeId + (++nodes));
                rc.setNodeId(nodeId);
                File fileOut = new File(outBaseDir, "node_"+nodeId+".txt");
                fileOut.delete();
                PrintStream filePrinter = new PrintStream(new FileOutputStream(fileOut), true);
                writer.println("\t"+dev+"\t===>\t"+fileOut.getAbsolutePath());
                writer.flush();

                c.initApp(rc, sc, tr, filePrinter);
            } catch (Throwable t) {
                writer.println("Error initializing device: "+dev);
                t.printStackTrace(writer);
                writer.flush();
            }
        }
        //now, update the routes in the test route configuration
        if ( testNodeCount > 0 ) {
            directConfiguration.dstlist.clear();
            routedConfiguration.routelist.clear();
            floodConfiguration.dstlist.clear();
            //for each node create one longest route
            for ( byte i = 0; i < testNodeCount; i ++ ) {
                ArrayList<Byte> route = new ArrayList<Byte>();
                for ( byte j = 0; j < testNodeCount; j ++ )
                    if ( i != j ) {
                        route.add((byte)(baseNodeId + 1+ j));
                    }
                byte dstNode = (byte)(baseNodeId + 1 + i);
                route.add(dstNode);
                routedConfiguration.routelist.add(route);
                writer.println("[Routed] Route added: "+route);
                directConfiguration.dstlist.add(dstNode);
                writer.println("[Direct] Destination added: "+dstNode);
                floodConfiguration.dstlist.add(dstNode);
                writer.println("[Flood] Destination added: "+dstNode);
                writer.println();
            }
        }
        return result;
    }

}
