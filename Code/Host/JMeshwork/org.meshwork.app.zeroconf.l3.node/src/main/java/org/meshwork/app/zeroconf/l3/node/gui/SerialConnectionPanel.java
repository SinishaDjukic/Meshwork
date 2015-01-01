package org.meshwork.app.zeroconf.l3.node.gui;

import com.prosyst.pgui.ImageIcon;
import com.prosyst.pgui.PComboBox;
import com.prosyst.pgui.PImageButton;
import com.prosyst.pgui.PPanel;
import com.prosyst.pgui.io.IniFile;
import jssc.SerialPortList;
import org.meshwork.core.transport.serial.jssc.SerialConfiguration;
import org.meshwork.core.transport.serial.jssc.SerialMessageTransport;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.FileInputStream;
import java.util.Vector;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public class SerialConnectionPanel extends PPanel implements AbstractElement, ActionListener {

    public final ImageIcon IMG_REFRESH = new ImageIcon(getClass().getResourceAsStream("res/refresh.png"));
    public final ImageIcon IMG_CONNECTED = new ImageIcon(getClass().getResourceAsStream("res/connected.png"));
    public final ImageIcon IMG_DISCONNECTED = new ImageIcon(getClass().getResourceAsStream("res/disconnected.png"));
    public final ImageIcon IMG_APPLY = new ImageIcon(getClass().getResourceAsStream("res/apply.png"));

    public static final int ACTION_CONNECTED = 0;
    public static final int ACTION_DISCONNECTED = 1;
    public static final int ACTION_APPLY = 3;

    public static final Integer[] SPEED_LIST = { 115200, 9600 };
    public static final int SPEED_DEFAULT = 0;
    protected static final String CFG_SERIAL_FILENAME = "serial.cfg";

    protected PImageButton portRefreshButton;
    protected PComboBox portCombo;
    protected PComboBox speedCombo;
    protected PImageButton connectButton;
    protected PImageButton applyButton;

    protected boolean connected = false;
    protected  Vector<ActionListener> listeners;
    protected SerialMessageTransport transport;

    public SerialConnectionPanel() {
        super();//new HorizontalFlowLayout());
        ((FlowLayout) getLayout()).setAlignment(FlowLayout.LEFT);
        listeners = new Vector<ActionListener>(2);
    }

    @Override
    public void init(Object context) {
        add(portCombo = new PComboBox());
        portCombo.setEditable(false);
        portCombo.addActionListener(this);

        add(portRefreshButton = new PImageButton(IMG_REFRESH));
        portRefreshButton.addActionListener(this);

        add(speedCombo = new PComboBox());
        speedCombo.setEditable(false);
        speedCombo.addActionListener(this);
        speedCombo.setListData(SPEED_LIST);
        speedCombo.setSelectedItemIndex(SPEED_DEFAULT);

        add(connectButton = new PImageButton(IMG_DISCONNECTED));
        connectButton.addActionListener(this);

        add(applyButton = new PImageButton(IMG_APPLY));
        applyButton.addActionListener(this);
        applyButton.setEnabled(connected);

        refreshPorts();
    }

    public void refreshPorts() {
        String[] portNames = SerialPortList.getPortNames();
        portCombo.setListData(portNames);
        if ( portNames == null || portNames.length == 0 ) {
            portCombo.setEditorValue("<No ports found>");
        } else {
            portCombo.setSelectedItemIndex(0);
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
            String portName = (String) portCombo.getEditorValue();
            SerialConfiguration config = createConfiguration();
            transport.init(config, portName);
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

    protected SerialConfiguration createConfiguration() {
        //We go for the below process since we don't want to expose all serial settings in the UI

        //Step 1: set current baud rate with some default values first
        SerialConfiguration config = new SerialConfiguration(115200, 8, 2, 0, false, false);
//            public SerialConfiguration(int baudRate, int dataBits, int stopBits, int parity, boolean setRTS, boolean setDTR) {
        //Step 2: then apply the override file
        try {
            config.loadConfiguration(new FileInputStream(CFG_SERIAL_FILENAME));
        } catch (Throwable t) {
            System.out.println("Note: no serial configuration file applied");
        }
        //Step 3: then apply the selected baud rate
        config.setBaudRate(Integer.valueOf((String) speedCombo.getEditorValue()));
        return config;
    }

    @Override
    public Object getData() {
        SerialConnectionPanelData data = new SerialConnectionPanelData();
        data.port = String.valueOf(portCombo.getEditorValue());
        data.speed = Integer.valueOf((String)speedCombo.getEditorValue());
        data.connected = connected;
        return data;
    }

    @Override
    public void setData(Object data) {
        if ( data instanceof SerialConnectionPanelData ) {
            SerialConnectionPanelData d = (SerialConnectionPanelData) data;
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
