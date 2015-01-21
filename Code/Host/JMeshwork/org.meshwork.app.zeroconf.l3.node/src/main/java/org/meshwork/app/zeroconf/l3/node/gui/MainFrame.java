package org.meshwork.app.zeroconf.l3.node.gui;

import com.prosyst.pgui.*;
import com.prosyst.pgui.border.EmptyBorder;
import com.prosyst.pgui.io.IniFile;
import com.prosyst.pgui.layout.VerticalFlowLayout;
import com.prosyst.pgui.utils.filechooser.FileFilter;
import com.prosyst.pgui.utils.filechooser.PFileChooser;
import org.meshwork.core.transport.serial.jssc.SerialMessageTransport;
import org.meshwork.core.zeroconf.l3.MessageAdapter;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowEvent;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

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

    public final ImageIcon IMG_NEW = new ImageIcon(getClass().getResourceAsStream("res/new_16.png"));
    public final ImageIcon IMG_OPEN = new ImageIcon(getClass().getResourceAsStream("res/open_16.png"));
    public final ImageIcon IMG_SAVE = new ImageIcon(getClass().getResourceAsStream("res/save_16.png"));
    public final ImageIcon IMG_EXIT = new ImageIcon(getClass().getResourceAsStream("res/exit_16.png"));
    public final ImageIcon IMG_ABOUT = new ImageIcon(getClass().getResourceAsStream("res/about_16.png"));

    public static final String CMD_NEW = "New...";
    public static final String CMD_OPEN = "Open...";
    public static final String CMD_SAVE = "Save...";
    public static final String CMD_EXIT = "Exit";
    public static final String CMD_ABOUT = "About...";

    protected PMenuBar menuBar;
    protected AboutDialog aboutDialog;
    protected boolean aboutDialogLoaded;

    protected ArrayList<AbstractElement> elements;
    protected FileFilter[] fileFilter;

    protected PTextArea console;

    public MainFrame() {
    }

    @Override
    public void init(Object context) {
        elements = new ArrayList<AbstractElement>(10);

        fileFilter = new FileFilter[] {new FileFilter("L3 ZeroConf Files (*.zc3)", "zc3")};

        deliveryPanel = new DeliveryPanel();
        elements.add(deliveryPanel);
        deliveryPanel.init(this);

        networkCapabilitiesPanel = new NetworkCapabilitiesPanel();
        elements.add(networkCapabilitiesPanel);
        networkCapabilitiesPanel.init(this);

        networkConfigurationPanel = new NetworkConfigurationPanel();
        elements.add(networkConfigurationPanel);
        networkConfigurationPanel.init(this);

        reportingPanel = new ReportingPanel();
        elements.add(reportingPanel);
        reportingPanel.init(this);

        serialConnectionPanel = new SerialConnectionPanel();
        elements.add(serialConnectionPanel);
        serialConnectionPanel.init(this);

        serialNumberPanel = new SerialNumberPanel();
        elements.add(serialNumberPanel);
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
        pmain.setBorder(new EmptyBorder(0, 2, 0, 2));
        pmain.add(pleft, BorderLayout.WEST);
        pmain.add(pright, BorderLayout.CENTER);

        content.add(pmain, BorderLayout.WEST);

        Action menuItemFileNew = new ActionImpl(CMD_NEW, IMG_NEW);
        Action menuItemFileOpen = new ActionImpl(CMD_OPEN, IMG_OPEN);
        Action menuItemFileSave = new ActionImpl(CMD_SAVE, IMG_SAVE);
        Action menuItemFileExit = new ActionImpl(CMD_EXIT, IMG_EXIT);
        Action menuItemHelpAbout = new ActionImpl(CMD_ABOUT, IMG_ABOUT);

        menuBar = new PMenuBar();
        PMenu menuFile = menuBar.addMenu("File");
        menuFile.setMnemonic('F');
        PMenuItem item;

        item = menuFile.add(menuItemFileNew);
        item.addActionListener(this);
        item.setMnemonic('N');
        item.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N, KeyEvent.CTRL_MASK, false));

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

        console = new PTextArea(false, 80);
        Font consoleFont = console.getFont();
        consoleFont = new Font(Font.MONOSPACED, Font.PLAIN, consoleFont.getSize());
        console.setFont(consoleFont);
        PScrollPane consoleScroll = new PScrollPane(console);
        content.add(consoleScroll, BorderLayout.CENTER);

        GUILogger.setInstance(new GUILogger(console));

        updateTitle(false, null);
        //setResizable(false);

        //calculate layout size
        revalidate();
        pack();

        //center on screen
        Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension size = getSize();
        setLocation((screen.width - size.width)/2, (screen.height - size.height)/2);

        ImageIcon logoIcon = new ImageIcon(getClass().getResource("res/meshwork_16.png"));
        setIconImage(logoIcon.getImage());
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
        elements.clear();
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
                    applyDeviceConfiguration(serialConnectionPanel.getAdapter(), serialConnectionPanel.getTransport());
                    break;
                case SerialConnectionPanel.ACTION_DISCONNECTING:
                    disconnectDevice(serialConnectionPanel.getAdapter(), serialConnectionPanel.getTransport());
                    break;
                case SerialConnectionPanel.ACTION_DISCONNECTED:
                    updateTitle(false, null);
                    break;
                case SerialConnectionPanel.ACTION_CONNECTED:
                    updateTitle(true, ((SerialConnectionData)serialConnectionPanel.getData()));
                    readDeviceConfiguration(serialConnectionPanel.getAdapter(), serialConnectionPanel.getTransport());
                    break;
            }
        } else if (CMD_EXIT.equals(action)) {
            exit();
        } else if (CMD_NEW.equals(action)) {
            resetPanels();
        } else if (CMD_OPEN.equals(action)) {
            open();
        } else if (CMD_SAVE.equals(action)) {
            save();
        } else if (CMD_ABOUT.equals(action)) {
            about();
        }
    }

    protected void updateTitle(boolean connected, SerialConnectionData data) {
        setTitle("L3 ZeroConf" + (connected ? (" @ " + data.port) : ""));
    }

    protected void clearConsole() {
        console.setText(null);
    }

    protected void open() {
        PFileChooser fc = PFileChooser.getInstance(this);
        fc.setFileFilters(fileFilter);
        String [] browseFile = fc.showFileDialog("Open configuration", false, true);
        String file = browseFile != null && browseFile.length > 0 ? browseFile[0] : null;
        if ( file != null ) {
            try {
                File f = new File(file);
                if (f.exists()) {
                    openConfiguration(f);
                }
            } catch (Throwable t) {
                System.err.println("[ERROR] Opening configuration file failed: "+t);
            }
        }
    }

    protected void save() {
        PFileChooser fc = PFileChooser.getInstance(this);
        fc.setFileFilters(fileFilter);
        String[] browseFile = fc.showFileDialog("Save configuration", false);
        String file = browseFile != null && browseFile.length > 0 ? browseFile[0] : null;
        if (file != null) {
            try {
                File f = new File(file);
                if (f.exists()) {
                    int answer = POptionPane.showConfirmDialog(this, "Overwrite file?\n"+f.getCanonicalPath(), "File exists", POptionPane.YES_NO_OPTION);
                    if (answer == 0) {//yes
                        //fall through
                    } else {//no
                        return;
                    }
                }
                saveConfiguration(f);
            } catch (Throwable t) {
                System.err.println("[ERROR] Saving configuration file failed: " + t);
            }
        }
    }

    protected void about() {
        if (!aboutDialogLoaded) {
            aboutDialog = new AboutDialog(this);
            aboutDialogLoaded = true;
        }
        if (aboutDialog.initFlag)
            aboutDialog.init(this);
        aboutDialog.setLocationRelativeTo(this);
        aboutDialog.setVisible(true);
    }

    protected void openConfiguration(File f) throws IOException {
        IniFile ini = new IniFile();
        ini.load(f);

        for (AbstractElement element:elements) {
            element.read(ini);
        }
    }

    protected void saveConfiguration(File f) throws IOException {
        IniFile ini = new IniFile();
        for (AbstractElement element:elements) {
            element.write(ini);
        }
        ini.save(f);
    }

    protected void applyDeviceConfiguration(MessageAdapter adapter, SerialMessageTransport transport) {
        WriteDeviceTask writeDeviceTask = new WriteDeviceTask(adapter, transport);
        UpdatePanelsTask updatePanelsTask = new UpdatePanelsTask(this);
        ArrayList<AbstractTask> tasks = new ArrayList<AbstractTask>();
        writeDeviceTask.setInput(getPanelData());
        tasks.add(writeDeviceTask);
        tasks.add(updatePanelsTask);
        TaskMonitor taskMonitor = new TaskMonitor(tasks);
        taskMonitor.start();
    }

    protected void readDeviceConfiguration(MessageAdapter adapter, SerialMessageTransport transport) {
        ReadDeviceTask readDeviceTask = new ReadDeviceTask(adapter, transport);
        UpdatePanelsTask updatePanelsTask = new UpdatePanelsTask(this);
        ArrayList<AbstractTask> tasks = new ArrayList<AbstractTask>();
        tasks.add(readDeviceTask);
        tasks.add(updatePanelsTask);
        TaskMonitor taskMonitor = new TaskMonitor(tasks);
        taskMonitor.start();
    }

    protected void disconnectDevice(MessageAdapter adapter, SerialMessageTransport transport) {
        DisconnectDeviceTask disconnectDeviceTask = new DisconnectDeviceTask(adapter, transport);

        //Option 1: Run in a separate thread and make sure the task deinits the transport
        ArrayList<AbstractTask> tasks = new ArrayList<AbstractTask>();
        tasks.add(disconnectDeviceTask);
        TaskMonitor taskMonitor = new TaskMonitor(tasks);
        taskMonitor.start();

        //Option 2: Calling synchronously while the transport is still open
//        try {
//            disconnectDeviceTask.runImpl();
//        } catch (Throwable t) {
//            GUILogger.error("Error disconnecting device due to: "+t.getMessage(), t);
//        }
    }

    public void resetPanels() {
        ArrayList<AbstractData> input = new ArrayList<AbstractData>();
        DeliveryData deliveryData = new DeliveryData();
        input.add(deliveryData);
        NetworkCapabilitiesData networkCapabilitiesData = new NetworkCapabilitiesData();
        input.add(networkCapabilitiesData);
        NetworkConfigurationData networkConfigurationData = new NetworkConfigurationData();
        input.add(networkConfigurationData);
        ReportingData reportingData = new ReportingData();
        input.add(reportingData);
        SerialNumberData serialNumberData = new SerialNumberData();
        input.add(serialNumberData);
        updatePanels(input);
    }

    public void updatePanels(ArrayList<AbstractData> input) {
        if ( input != null ) {
            for (AbstractData data : input) {
                if ( data instanceof DeliveryData ) {
                    deliveryPanel.setData((DeliveryData)data);
                } else if ( data instanceof NetworkCapabilitiesData ) {
                    networkCapabilitiesPanel.setData((NetworkCapabilitiesData)data);
                } else if ( data instanceof NetworkConfigurationData ) {
                    networkConfigurationPanel.setData((NetworkConfigurationData)data);
                } else if ( data instanceof ReportingData ) {
                    reportingPanel.setData((ReportingData )data);
                } else if ( data instanceof SerialNumberData ) {
                    serialNumberPanel.setData((SerialNumberData)data);
                }
            }
        }
    }

    public ArrayList<AbstractData> getPanelData() {
        ArrayList<AbstractData> result = new ArrayList<AbstractData>();
        result.add((AbstractData)deliveryPanel.getData());
        result.add((AbstractData)networkCapabilitiesPanel.getData());
        result.add((AbstractData)networkConfigurationPanel.getData());
        result.add((AbstractData)reportingPanel.getData());
        result.add((AbstractData)serialNumberPanel.getData());
        return result;
    }
}