package org.meshwork.app.zeroconf.l3.node.gui;

import com.prosyst.pgui.PTextArea;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by Sinisha Djukic on 2.1.2015.
 */
public class GUILogger {

    protected static GUILogger instance;
    protected static SimpleDateFormat timeFormat = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss");

    protected PTextArea console;

    public GUILogger(PTextArea console) {
        this.console = console;
    }

    protected void infoImpl(String message) {
        String time = timeFormat.format(new Date(System.currentTimeMillis()));
        console.append("<"+time+"> [INFO] "+message+"\n");
    }

    protected void warningImpl(String message, Throwable t) {
        String time = timeFormat.format(new Date(System.currentTimeMillis()));
        console.append("<"+time+"> [WARN] "+message+"\n... "+t+"\n");
    }

    protected void errorImpl(String message, Throwable t) {
        String time = timeFormat.format(new Date(System.currentTimeMillis()));
        console.append("<"+time+"> [ERR ] "+message+"\n... Exception: "+t+"\n");
    }

    protected void clearImpl() {
        console.setText(null);
    }

    public static void info(String message) {
        getInstance().infoImpl(message);
    }

    protected void warning(String message, Throwable t) {
        getInstance().warningImpl(message, t);
    }

    protected static void error(String message, Throwable t) {
        getInstance().errorImpl(message, t);
    }

    protected static void clear() {
        getInstance().clearImpl();
    }

    public static GUILogger getInstance() {
        return instance;
    }

    public static void setInstance(GUILogger logger) {
        instance = logger;
    }
}
