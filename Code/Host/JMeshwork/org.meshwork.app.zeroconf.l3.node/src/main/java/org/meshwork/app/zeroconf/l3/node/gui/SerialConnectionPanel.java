package org.meshwork.app.zeroconf.l3.node.gui;

import com.prosyst.pgui.*;
import com.prosyst.pgui.combobox.ComboBoxEditorImpl;
import com.prosyst.pgui.io.IniFile;
import jssc.SerialPortList;
import org.meshwork.core.transport.serial.jssc.SerialConfiguration;
import org.meshwork.core.transport.serial.jssc.SerialMessageTransport;
import org.meshwork.core.zeroconf.l3.MessageAdapter;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.FileInputStream;
import java.util.Vector;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public class SerialConnectionPanel extends PPanel implements AbstractElement, ActionListener {

    public final ImageIcon IMG_REFRESH = new ImageIcon(getClass().getResourceAsStream("res/refresh_16.png"));
    public final ImageIcon IMG_CONNECTED = new ImageIcon(getClass().getResourceAsStream("res/connected_16.png"));
    public final ImageIcon IMG_DISCONNECTED = new ImageIcon(getClass().getResourceAsStream("res/disconnected_16.png"));
    public final ImageIcon IMG_APPLY = new ImageIcon(getClass().getResourceAsStream("res/apply_16.png"));

    public static final int ACTION_CONNECTED = 0;
    public static final int ACTION_DISCONNECTED = 1;
    public static final int ACTION_APPLY = 3;

    public static final Integer[] SPEED_LIST = { 115200, 57600, 38400, 19200, 9600 };
    public static final int SPEED_DEFAULT = 0;
    protected static final String CFG_SERIAL_FILENAME = "serial.cfg";

    protected PComboBox portCombo;
    protected PButton portRefreshButton;
    protected PComboBox speedCombo;
    protected PButton connectButton;
    protected PButton applyButton;

    protected boolean connected = false;
    protected  Vector<ActionListener> listeners;
    protected SerialMessageTransport transport;
    protected MainFrame mainFrame;
    protected MessageAdapter adapter;

    public SerialConnectionPanel() {
        super();//new HorizontalFlowLayout());
        ((FlowLayout) getLayout()).setAlignment(FlowLayout.LEFT);
        listeners = new Vector<ActionListener>(2);
    }

    @Override
    public void init(Object context) {
        mainFrame = (MainFrame) context;
        add(portCombo = new PComboBox());
        ((ComboBoxEditorImpl)portCombo.getComboBoxEditor()).setColumns(30);
        portCombo.setEditable(false);
        portCombo.addActionListener(this);

        add(portRefreshButton = createButton(IMG_REFRESH));
        portRefreshButton.addActionListener(this);
        portRefreshButton.setToolTipText("Refresh ports");

        PPanel spacer1 = new PPanel();
        spacer1.setPreferredSize(new Dimension(5, 1));
        add(spacer1);

        add(speedCombo = new PComboBox());
        ((ComboBoxEditorImpl)portCombo.getComboBoxEditor()).setColumns(10);
        speedCombo.setEditable(true);
        ((PTextField)speedCombo.getComboBoxEditor()).setRadix(PTextField.DEC);
        speedCombo.addActionListener(this);
        speedCombo.setListData(SPEED_LIST);
        speedCombo.setSelectedItemIndex(SPEED_DEFAULT);

        add(connectButton = createButton(null));
        setConnectState(false);
        connectButton.addActionListener(this);

        PPanel spacer2 = new PPanel();
        spacer2.setPreferredSize(new Dimension(5, 1));
        add(spacer2);

        add(applyButton = createButton(IMG_APPLY));
        applyButton.addActionListener(this);
        applyButton.setEnabled(connected);
        applyButton.setToolTipText("Apply configuration");

        refreshPorts();
    }

    protected PButton createButton(ImageIcon icon) {
        PButton button = new PButton(icon);
//        button.setRequestFocusEnabled(false);
//        button.setFocusPainted(false);
        button.setPressedBorder(null);
        button.setBorder(null);
        button.setBorderPainted(false);
        button.setOpaque(false);
        return button;
    }

    public void refreshPorts() {
        String[] portNames = SerialPortList.getPortNames();
        portCombo.setListData(portNames);
        if ( portNames == null || portNames.length == 0 ) {
            portCombo.setEditorValue("<No ports found>");
            connectButton.setEnabled(false);
        } else {
            portCombo.setSelectedItemIndex(0);
            connectButton.setEnabled(true);
        }
    }

    @Override
    public void deinit(Object context) {
        portCombo.removeActionListener(this);
        portRefreshButton.removeActionListener(this);
        speedCombo.removeActionListener(this);
        connectButton.removeActionListener(this);
        applyButton.removeActionListener(this);
        removeAll();
        mainFrame = null;
        adapter = null;
        transport = null;
    }
    
    public void addActionListener(ActionListener al) {
        listeners.add(al);
    }

    protected void fireActionEvent(ActionEvent ae) {
        synchronized (listeners) {
            for (ActionListener al : listeners) {
                try {
                    al.actionPerformed(ae);
                } catch (Throwable t) {
                    System.err.println("Error firing ActionEvent: " + ae + ", due to: " + t);
                    t.printStackTrace();
                }
            }
        }
    }
    
    public void removeActionListener(ActionListener al) {
        listeners.remove(al);
    }

    public void connect(boolean b) throws Exception {
        if ( b == connected )
            return;
        if ( b ) {
            transport = new SerialMessageTransport();
            adapter = new MessageAdapter();
            String portName = (String) portCombo.getEditorValue();
            SerialConfiguration config = createConfiguration();
            if ( config == null ) {
                POptionPane.showMessageDialog(mainFrame, "Invalid baud rate entered", "Error", POptionPane.ERROR_MESSAGE);
                return;
            } else {
                transport.init(config, portName);
            }
        } else {
            try {
                transport.deinit();
            } catch (Throwable t) {
                System.err.println("[Warning] Error during transport deinit: "+t);
            }
            transport = null;
        }
        setConnectState(b);
        ActionEvent ae = new ActionEvent(this, b ? ACTION_CONNECTED : ACTION_DISCONNECTED, null);
        fireActionEvent(ae);
    }

    public SerialMessageTransport getTransport() {
        return transport;
    }

    public MessageAdapter getAdapter() {
        return adapter;
    }

    protected SerialConfiguration createConfiguration() {
        //We go for the below process since we don't want to expose all serial settings in the UI

        //Step 0: Sanity check for user-entered value
        Integer speed = Integer.valueOf((String) speedCombo.getEditorValue());
        if ( speed < 1 ) {
            return null;
        }

        //Step 1: set current baud rate with some default values first
        SerialConfiguration config = new SerialConfiguration(115200, 8, 2, 0, false, false);

        //Step 2: then apply the override file
        try {
            config.loadConfiguration(new FileInputStream(CFG_SERIAL_FILENAME));
        } catch (Throwable t) {
            System.out.println("Note: no serial configuration file applied");
        }

        //Step 3: then apply the selected baud rate
        config.setBaudRate(speed);
        return config;
    }

    @Override
    public Object getData() {
        SerialConnectionData data = new SerialConnectionData();
        data.port = String.valueOf(portCombo.getEditorValue());
        data.speed = Integer.valueOf((String)speedCombo.getEditorValue());
        data.connected = connected;
        return data;
    }

    @Override
    public void setData(Object data) {
        if ( data instanceof SerialConnectionData) {
            SerialConnectionData d = (SerialConnectionData) data;
            portCombo.setEditorValue(d.port);
            speedCombo.setEditorValue(d.speed);
            setConnectState(d.connected);
        }
    }

    public boolean isConnected() {
        return connected;
    }

    private void setConnectState(boolean connected) {
        this.connected = connected;
        connectButton.setIcon(connected ? IMG_CONNECTED : IMG_DISCONNECTED);
        connectButton.setToolTipText(connected ? "Disconnect" : "Connect");
    }

    @Override
    public void read(IniFile ini) {
        //Nothing to persist
    }

    @Override
    public void write(IniFile ini) {
        //Nothing to persist
    }

    protected void apply() {
        ActionEvent ae = new ActionEvent(this, ACTION_APPLY, null);
        fireActionEvent(ae);
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        Object src = e.getSource();
        if (src == portRefreshButton) {
            refreshPorts();
//        } else if ( src == portCombo ) {
//        } else if ( src == connectButton ) {
        } else if (src == connectButton) {
            try {
                connect(!connected);
            } catch (Exception e1) {
                System.err.println("Error during serial connection due to: " + e1);
                e1.printStackTrace();
            }
            applyButton.setEnabled(connected);
        } else if (src == applyButton) {
            apply();
        }
    }

}
