package org.meshwork.app.host.l3.router.serial;

import org.meshwork.app.host.l3.router.Router;
import org.meshwork.app.host.l3.router.RouterConfiguration;
import org.meshwork.core.AbstractMessageTransport;
import org.meshwork.core.transport.serial.jssc.SerialConfiguration;
import org.meshwork.core.transport.serial.jssc.SerialMessageTransport;

import java.io.*;

/**
 * Created by Sinisha Djukic on 14-2-12.
 */
public class Console {

    public static final int EXIT_OK                             =  0;
    public static final int EXIT_USAGE                          = 10;
    public static final int EXIT_ROUTER_CONFIG_FILE_ERROR       = 20;
    public static final int EXIT_SERIAL_CONFIG_FILE_ERROR       = 30;
    public static final int EXIT_INITIALIZATION_ERROR           = 40;


    public static void main(String[] args) {
        System.out.println("L3 Router Host App");
        System.out.println("Copyleft 2014, Meshwork Project, Sinisha Djukic");
        if ( args == null || args.length == 0 || "-h".equalsIgnoreCase(args[0]) || "-help".equalsIgnoreCase(args[0]) || args.length != 3 ) {
            printUsage();
            exit(EXIT_USAGE, null);
        }
        try {
            RouterConfiguration routerConfig = initRouterConfiguration(args[0].trim());
            SerialConfiguration serialConfig = initSerialConfiguration(args[1].trim());
            AbstractMessageTransport transport = initTransport(serialConfig, args[2]);
            Router router = new Router();
            router.init(transport, routerConfig, System.out);
        } catch (Throwable t) {
            exit(EXIT_INITIALIZATION_ERROR, "Error during router initialization due to: "+t.getMessage());
        }
        System.out.println("Router started!");
        System.out.println();
    }

    private static SerialMessageTransport initTransport(SerialConfiguration serialConfig, String port) throws Exception {
        SerialMessageTransport result = new SerialMessageTransport();
        result.init(serialConfig, port);
        return result;
    }

    private static RouterConfiguration initRouterConfiguration(String s_config) {
        RouterConfiguration result = null;
        FileInputStream fis = null;
        try {
            File f = new File(s_config);
            if ( f.exists() ) {
                fis = new FileInputStream(f);
                RouterConfiguration temp = new RouterConfiguration();
                temp.loadConfiguration(fis);
                result = temp;
            }
        } catch (Throwable t) {
            exit(EXIT_ROUTER_CONFIG_FILE_ERROR, "Error reading router configuration file '"+s_config+"' due to: "+t.getMessage());
        } finally {
            if ( fis != null )
                closeSilently(fis);
        }
        return result;
    }

    private static SerialConfiguration initSerialConfiguration(String config) {
        SerialConfiguration result = null;
        FileInputStream fis = null;
        try {
            File f = new File(config);
            if ( f.exists() ) {
                fis = new FileInputStream(f);
                SerialConfiguration temp = new SerialConfiguration();
                temp.loadConfiguration(fis);
                result = temp;
            }
        } catch (Throwable t) {
            exit(EXIT_SERIAL_CONFIG_FILE_ERROR, "Error reading serial configuration file '"+config+"' due to: "+t.getMessage());
        } finally {
            if ( fis != null )
                closeSilently(fis);
        }
        return result;
    }

    private static void closeSilently(InputStream is) {
        try {
            is.close();
        } catch (IOException e) {
        }
    }

    private static void exit(int i, String s) {
        if ( s != null )
            System.out.println(s);
        System.exit(i);
    }

    private static void printUsage() {
        System.out.println("Usage: <router config> <serial config> <serial device>");
        System.out.println("       <router config>=<file name>");
        System.out.println("       <serial config>=<file name>");
        System.out.println("       <serial device>=<device name/path");
        System.out.println("Example: router.cfg serial.cfg /dev/ttyUSB0");
        System.out.println("Note that both router and serial configs can point to the same file.");
        System.out.println();
    }

}
