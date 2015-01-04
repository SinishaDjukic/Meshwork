package org.meshwork.app.zeroconf.l3.node.gui;

import java.util.ArrayList;

/**
 * Created by Sinisha Djukic on 2.1.2015.
 */
public abstract class AbstractTask implements Runnable {

    public static final int NOT_STARTED = 0;
    public static final int RUNNING = 1;
    public static final int FINISHED = 2;

    protected int state;
    protected ArrayList<AbstractData> input;
    protected ArrayList<AbstractData> output;
    protected Throwable exception;
    protected String name;

    public AbstractTask(String name) {
        this.name = name;
        state = NOT_STARTED;;
    }

    public String getName() {
        return name;
    }

    public int getState() {
        return state;
    }

    public ArrayList<AbstractData> getOutput() {
        return output;
    }

    public ArrayList<AbstractData> getInput() {
        return input;
    }

    public void setInput(ArrayList<AbstractData> input) {
        this.input = input;
    }

    public Throwable getException() {
        return exception;
    }

    @Override
    public void run() {
        synchronized (this) {
            state = RUNNING;
            try {
                output = runImpl();
            } catch (Throwable t) {
                exception = t;
            }
            state = FINISHED;
            notifyAll();
        }
    }

    public abstract ArrayList<AbstractData> runImpl() throws Throwable;

}