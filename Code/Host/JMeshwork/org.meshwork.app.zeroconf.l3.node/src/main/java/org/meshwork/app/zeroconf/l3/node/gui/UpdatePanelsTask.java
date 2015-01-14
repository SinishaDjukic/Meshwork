package org.meshwork.app.zeroconf.l3.node.gui;

import java.util.ArrayList;

/**
 * Created by Sinisha Djukic on 3.1.2015.
 */
public class UpdatePanelsTask extends AbstractTask {

    protected MainFrame mainFrame;

    public UpdatePanelsTask(MainFrame mainFrame) {
        super("Update Panels Task");
        this.mainFrame = mainFrame;
    }

    @Override
    public ArrayList<AbstractData> runImpl() throws Throwable {
        ArrayList<AbstractData> in = getInput();
        GUILogger.info("[UpdatePanelsTask] Input data: "+in);
        mainFrame.updatePanels(in);
        return null;
    }
}