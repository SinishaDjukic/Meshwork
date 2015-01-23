package org.meshwork.app.zeroconf.l3.node.gui;

import java.util.ArrayList;

/**
 * Created by Sinisha Djukic on 3.1.2015.
 */
public class UpdateUITask extends AbstractTask {

    public static final int ACTION_UPDATE_PANELS = 0;
    public static final int ACTION_SET_DISCONNECTED = 1;

    protected int action;
    protected MainFrame mainFrame;

    public UpdateUITask(MainFrame mainFrame, int action) {
        super("Update Panels Task");
        this.mainFrame = mainFrame;
        this.action = action;
    }

    @Override
    public ArrayList<AbstractData> runImpl() throws Throwable {
        switch (action) {
            case ACTION_UPDATE_PANELS:
                ArrayList<AbstractData> in = getInput();
                GUILogger.info("[UpdateUITask] Input data: "+in);
                mainFrame.updatePanels(in);
                break;
            case ACTION_SET_DISCONNECTED:
                mainFrame.updateConnectionState(false);
                break;
        }
        return null;
    }
}