package org.meshwork.app.host.l3.router.console;

import org.meshwork.app.host.l3.router.MessageDispatcherImpl;
import org.meshwork.app.host.l3.router.Router;
import org.meshwork.app.host.l3.router.RouterConfiguration;
import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.host.l3.MessageAdapter;
import org.meshwork.core.transport.serial.jssc.SerialConfiguration;
import org.meshwork.core.transport.serial.jssc.SerialMessageTransport;

import java.io.*;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public abstract class AbstractConsole {

    public static final int EXIT_OK                             =  0;
    public static final int EXIT_USAGE                          = 10;
    public static final int EXIT_CONFIG_FILE_ERROR              = 20;
    public static final int EXIT_INITIALIZATION_ERROR           = 30;

    public String[] args;

    public static void closeSilently(InputStream is) {
        try {
            is.close();
        } catch (IOException e) {
        }
    }

    public static void exit(int i, String s) {
        if ( s != null )
            System.out.println(s);
        System.exit(i);
    }

    protected abstract void printAppHeader();

    protected void printUsage() {
        System.out.println("Usage: <router config> <serial config> <serial device>");
        System.out.println("       <router config>=<file name>");
        System.out.println("       <serial config>=<file name>");
        System.out.println("       <serial device>=<device name/path");
        System.out.println("Example: router.cfg serial.cfg /dev/ttyUSB0");
        System.out.println("Note that all configs can point to the same file.");
        System.out.println();
    }

    protected void checkParams(String[] args) {
        if ( args == null || args.length == 0 || "-h".equalsIgnoreCase(args[0]) || "-help".equalsIgnoreCase(args[0]) || args.length != 3 ) {
            printUsage();
            exit(EXIT_USAGE, null);
        }
    }

    public void init(String[] args) {
        printAppHeader();
        checkParams(args);
        initApp(args);
        printStartedMessage();
    }

    protected abstract void printStartedMessage();

    protected void initApp(String[] args) {
        try {
            this.args = args;
            RouterConfiguration routerConfig = initRouterConfiguration(args[0].trim());
            SerialConfiguration serialConfig = initSerialConfiguration(args[1].trim());
            AbstractMessageTransport transport = initTransport(serialConfig, args[args.length-1]);
            initApp(routerConfig, serialConfig, transport, System.out);
        } catch (Throwable t) {
            t.printStackTrace();
            exit(EXIT_INITIALIZATION_ERROR, "Error during router initialization due to: "+t.getMessage());
        }
    }

    protected void initApp(RouterConfiguration routerConfig, SerialConfiguration serialConfig, AbstractMessageTransport transport, PrintStream ps) throws Exception {
        MessageAdapter adapter = initMessageAdapter();
        PrintWriter writer = initPrintWriter(ps, true);
        MessageDispatcherImpl dispatcher = initMessageDispatcher(adapter, transport, routerConfig, writer);
        Router router = initRouter();
        router.init(transport, dispatcher, routerConfig, writer);
    }

    protected PrintWriter initPrintWriter(PrintStream out, boolean b) {
        return new PrintWriter(out, true);
//        return new IndentedPrintWriter(out, true, " ");
    }

    protected Router initRouter() {
        return new Router();
    }

    protected abstract MessageDispatcherImpl initMessageDispatcher(MessageAdapter adapter, AbstractMessageTransport transport, RouterConfiguration routerConfig, PrintWriter writer) throws Exception;

    protected MessageAdapter initMessageAdapter() {
        return new MessageAdapter();
    }

    public static InputStream getInputStream(String configFile) {
        FileInputStream fis = null;
        try {
            File f = new File(configFile);
            if ( f.exists() )
                fis = new FileInputStream(f);
        } catch (Throwable t) {
            exit(EXIT_CONFIG_FILE_ERROR, "Error reading configuration file '"+configFile+"' due to: "+t.getMessage());
        }
        return fis;
    }

    protected SerialMessageTransport initTransport(SerialConfiguration serialConfig, String port) throws Exception {
        SerialMessageTransport result = new SerialMessageTransport();
        result.init(serialConfig, port);
        return result;
    }

    protected RouterConfiguration initRouterConfiguration(String config) throws Exception {
        RouterConfiguration result = null;
        InputStream is = null;
        try {
            result = new RouterConfiguration();
            result.loadConfiguration(is = getInputStream(config));
        } catch (Throwable t) {
            exit(EXIT_CONFIG_FILE_ERROR, "Error parsing router configuration file '"+config+"' due to: "+t.getMessage());
        } finally {
            closeSilently(is);
        }
        return result;
    }

    protected SerialConfiguration initSerialConfiguration(String config) throws Exception {
        SerialConfiguration result = null;
        InputStream is = null;
        try {
            result = new SerialConfiguration();
            result.loadConfiguration(is = getInputStream(config));
        } catch (Throwable t) {
            exit(EXIT_CONFIG_FILE_ERROR, "Error parsing serial configuration file '"+config+"' due to: "+t.getMessage());
        } finally {
            closeSilently(is);
        }
        return result;
    }
}
