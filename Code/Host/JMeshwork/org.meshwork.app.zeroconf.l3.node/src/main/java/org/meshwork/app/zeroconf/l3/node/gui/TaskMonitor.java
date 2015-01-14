package org.meshwork.app.zeroconf.l3.node.gui;

import java.util.ArrayList;

/**
 * Created by Sinisha Djukic on 2.1.2015.
 */
public class TaskMonitor implements Runnable {

    protected ArrayList<AbstractTask> tasks;

    public TaskMonitor(ArrayList<AbstractTask> tasks) {
        this.tasks = tasks;
    }

    @Override
    public void run() {
        GUILogger.clear();
        AbstractTask last = null;
        for (AbstractTask task: tasks) {
            GUILogger.info("Started: " + task.getName());
            if ( last != null )
                task.setInput(last.getOutput());
            task.run();
            last = task;
            Throwable exception = task.getException();
            if (exception != null) {
                GUILogger.error("Error running: " + task.getName(), exception);
                exception.printStackTrace(System.err);
                break;
            } else {
                GUILogger.info("Finished: " + task.getName());
            }
        }
    }

    public void start() {
        new Thread(this, "[MW] Task Monitor @ "+System.currentTimeMillis()).start();
    }
}
