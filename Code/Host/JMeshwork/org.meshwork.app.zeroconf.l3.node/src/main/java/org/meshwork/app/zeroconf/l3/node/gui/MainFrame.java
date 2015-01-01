package org.meshwork.app.zeroconf.l3.node.gui;

import com.prosyst.pgui.*;
import com.prosyst.pgui.io.IniFile;
import com.prosyst.pgui.layout.VerticalFlowLayout;
import org.meshwork.core.transport.serial.jssc.SerialMessageTransport;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowEvent;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public class MainFrame extends PFrame implements AbstractElement, ActionListener {

    protected DeliveryPanel deliveryPanel;
    protected NetworkCapabilitiesPanel networkCapabilitiesPanel;
    protected NetworkConfigurationPanel networkConfigurationPanel;
    protected ReportingPanel reportingPanel;
    protected SerialConnectionPanel serialConnectionPanel;
    protected SerialNumberPanel serialNumberPanel;

    public final ImageIcon IMG_OPEN = new ImageIcon(getClass().getResourceAsStream("res/open.png"));
    public final ImageIcon IMG_SAVE = new ImageIcon(getClass().getResourceAsStream("res/save.png"));
    public final ImageIcon IMG_EXIT = new ImageIcon(getClass().getResourceAsStream("res/exit.png"));
    public final ImageIcon IMG_ABOUT = new ImageIcon(getClass().getResourceAsStream("res/about.png"));

    public static final String CMD_OPEN = "Open...";
    public static final String CMD_SAVE = "Save...";
    public static final String CMD_EXIT = "Exit";
    public static final String CMD_ABOUT = "About...";

    protected PMenuBar menuBar;

    public MainFrame() {
    }

    @Override
    public void init(Object context) {
        deliveryPanel = new DeliveryPanel();
        deliveryPanel.init(this);
        networkCapabilitiesPanel = new NetworkCapabilitiesPanel();
        networkCapabilitiesPanel.init(this);
        networkConfigurationPanel = new NetworkConfigurationPanel();
        networkConfigurationPanel.init(this);
        reportingPanel = new ReportingPanel();
        reportingPanel.init(this);
        serialConnectionPanel = new SerialConnectionPanel();
        serialConnectionPanel.init(this);
        serialNumberPanel = new SerialNumberPanel();
        serialNumberPanel.init(this);

        PComponent content = (PComponent) getRootPane().getContentPane();
        content.add(serialConnectionPanel, BorderLayout.NORTH);
        serialConnectionPanel.addActionListener(this);

        PPanel pleft = new PPanel(new VerticalFlowLayout());
        pleft.add(networkCapabilitiesPanel);
        pleft.add(deliveryPanel);
        pleft.add(serialNumberPanel);

        PPanel pright = new PPanel(new VerticalFlowLayout());
        pright.add(networkConfigurationPanel);
        pright.add(reportingPanel);

        PPanel pmain = new PPanel();
        pmain.add(pleft, BorderLayout.WEST);
        pmain.add(pright, BorderLayout.CENTER);

        content.add(pmain, BorderLayout.CENTER);

        Action menuItemFileOpen = new ActionImpl(CMD_OPEN, IMG_OPEN);
        Action menuItemFileSave = new ActionImpl(CMD_SAVE, IMG_SAVE);
        Action menuItemFileExit = new ActionImpl(CMD_EXIT, IMG_EXIT);
        Action menuItemHelpAbout = new ActionImpl(CMD_ABOUT, IMG_ABOUT);

        menuBar = new PMenuBar();
        PMenu menuFile = menuBar.addMenu("File");
        menuFile.setMnemonic('F');
        PMenuItem item;
        item = menuFile.add(menuItemFileOpen);
        item.addActionListener(this);
        item.setMnemonic('O');
        item.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_O, KeyEvent.CTRL_MASK, false));
        item = menuFile.add(menuItemFileSave);
        item.addActionListener(this);
        item.setMnemonic('S');
        item.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, KeyEvent.CTRL_MASK, false));
        menuFile.addSeparator();
        item = menuFile.add(menuItemFileExit);
        item.addActionListener(this);
        item.setMnemonic('x');
        PMenu menuHelp = menuBar.addMenu("Help");
        menuHelp.setMnemonic('H');
        item = menuHelp.add(menuItemHelpAbout);
        item.addActionListener(this);
        item.setMnemonic('A');
        item.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F1, 0));
        getRootPane().setPMenuBar(menuBar);

        updateTitle(false, null);
        setResizable(false);

        //calculate layout size
        revalidate();
        pack();

        //center on screen
        Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension size = getSize();
        setLocation((screen.width - size.width)/2, (screen.height - size.height)/2);

        //show
        setVisible(true);
    }

    protected void processWindowEvent(WindowEvent e) {
        super.processWindowEvent(e);
        if (e.getID() == WindowEvent.WINDOW_CLOSING) {
            exit();
        }
    }

    protected void exit() {
        System.exit(0);
    }

    @Override
    public void deinit(Object context) {
        setVisible(false);
        deliveryPanel.deinit(this);
        networkCapabilitiesPanel.deinit(this);
        networkConfigurationPanel.deinit(this);
        reportingPanel.deinit(this);
        serialConnectionPanel.deinit(this);
        serialNumberPanel.deinit(this);
    }

    @Override
    public Object getData() {
        return null;
    }

    @Override
    public void setData(Object data) {

    }

    @Override
    public void read(IniFile ini) {

    }

    @Override
    public void write(IniFile ini) {

    }

    public static void main(String[] params) {
        MainFrame mainFrame = new MainFrame();
        mainFrame.init(null);
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        int id = e.getID();
        Object src = e.getSource();
        String action = e.getActionCommand();
        if ( src == serialConnectionPanel ) {
            switch (id) {
                case SerialConnectionPanel.ACTION_APPLY:
                    writeDeviceConfiguration(serialConnectionPanel.getTransport());
                    break;
                case SerialConnectionPanel.ACTION_DISCONNECTED:
                    updateTitle(false, null);
                    break;
                case SerialConnectionPanel.ACTION_CONNECTED:
                    updateTitle(true, ((SerialConnectionPanelData)serialConnectionPanel.getData()));
                    readDeviceConfiguration(serialConnectionPanel.getTransport());
                    break;
            }
        } else if (CMD_EXIT.equals(action)) {
            exit();
        } else if (CMD_OPEN.equals(action)) {
            open();
        } else if (CMD_SAVE.equals(action)) {
            save();
        } else if (CMD_ABOUT.equals(action)) {
            about();
        }
    }

    protected void updateTitle(boolean connected, SerialConnectionPanelData data) {
        setTitle("L3 ZeroConf" + (connected ? (" @ "+data.port) : ""));
    }

    protected void open() {
        //TODO implement open
        System.out.println("[TODO] open");
    }

    protected void save() {
        //TODO implement save
        System.out.println("[TODO] save");
    }

    protected void about() {
        //TODO implement about
        System.out.println("[TODO] about");
    }

    protected void writeDeviceConfiguration(SerialMessageTransport transport) {
        //TODO implement writing current device configuration
        System.out.println("[TODO] writeDeviceConfiguration");
    }

    protected void readDeviceConfiguration(SerialMessageTransport transport) {
        //TODO implement reading current device configuration
        System.out.println("[TODO] readDeviceConfiguration");
    }

}
