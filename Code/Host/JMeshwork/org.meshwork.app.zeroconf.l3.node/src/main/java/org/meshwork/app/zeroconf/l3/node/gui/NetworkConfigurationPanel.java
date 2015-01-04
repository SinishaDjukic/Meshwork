package org.meshwork.app.zeroconf.l3.node.gui;

import com.prosyst.pgui.*;
import com.prosyst.pgui.io.IniFile;
import com.prosyst.pgui.io.IniSection;
import com.prosyst.pgui.layout.GridBagConstraints2;
import org.meshwork.core.zeroconf.l3.Constants;

import java.awt.*;
import java.awt.event.TextEvent;
import java.awt.event.TextListener;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public class NetworkConfigurationPanel extends PTitledPanel implements AbstractElement, TextListener {

    public static final String INI_SECTION_NAME = "Network Configuration";
    public static final String INI_CHANNEL_KEY = "Channel";
    public static final String INI_NETWORKID_KEY = "Network ID";
    public static final String INI_NODEID_KEY = "Node ID";
    public static final String INI_NETWORKKEY_KEY = "Key";

    protected PScrollNumberField channelField;
    protected PScrollNumberField networkIDField;
    protected PScrollNumberField nodeIDField;
    protected PTextField lengthField;
    protected PTextField keyField;

    public NetworkConfigurationPanel() {
        super("Network Configuration");
    }

    @Override
    public void init(Object context) {
        containerPanel.setLayout(new GridBagLayout());

        //Channel
        containerPanel.add(new PLabel("Channel:"), new GridBagConstraints2(0, 0, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 0), 0, 3));
        containerPanel.add(channelField = new PScrollNumberField(), new GridBagConstraints2(1, 0, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 3), 0, 3));
        channelField.setMinimum(0);
        channelField.setMaximum(255);

        //Channel
        containerPanel.add(new PLabel("Network ID:"), new GridBagConstraints2(0, 1, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 0), 0, 3));
        containerPanel.add(networkIDField = new PScrollNumberField(), new GridBagConstraints2(1, 1, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 3), 0, 3));
        networkIDField.setMinimum(1);
        networkIDField.setMaximum(65535);

        //Channel
        containerPanel.add(new PLabel("Node ID:"), new GridBagConstraints2(0, 2, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 0), 0, 3));
        containerPanel.add(nodeIDField = new PScrollNumberField(), new GridBagConstraints2(1, 2, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 3), 0, 3));
        nodeIDField.setMinimum(Constants.MIN_NODE_ID);
        nodeIDField.setMaximum(Constants.MAX_NODE_ID);

        //Length
        containerPanel.add(new PLabel("Key Length:"), new GridBagConstraints2(0, 3, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 0), 0, 3));
        containerPanel.add(lengthField = new PTextField(), new GridBagConstraints2(1, 3, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 3), 0, 3));
        lengthField.setEditable(false);

        //Key
        containerPanel.add(new PLabel("Key:"), new GridBagConstraints2(0, 4, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 0), 0, 3));
        containerPanel.add(keyField = new PTextField(), new GridBagConstraints2(1, 4, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 3), 0, 3));

        keyField.addTextListener(this);
        recalculateTextLength();
    }

    @Override
    public void deinit(Object context) {
        containerPanel.removeAll();
        keyField.removeTextListener(this);
    }

    @Override
    public Object getData() {
        NetworkConfigurationData data = new NetworkConfigurationData();
        data.channelID = (byte) channelField.getValue();
        data.networkID = (byte) networkIDField.getValue();
        data.nodeID = (byte) nodeIDField.getValue();
        data.networkKey = keyField.getText();
        return data;
    }

    @Override
    public void setData(Object data) {
        if ( data instanceof NetworkConfigurationData) {
            NetworkConfigurationData d = (NetworkConfigurationData) data;
            channelField.setValue(d.channelID);
            networkIDField.setValue(d.networkID);
            nodeIDField.setValue(d.nodeID);
            keyField.setText(d.networkKey);
        }
    }

    @Override
    public void setEnabled(boolean enabled) {
        channelField.setEnabled(enabled);
        networkIDField.setEnabled(enabled);
        nodeIDField.setEnabled(enabled);
        lengthField.setEnabled(enabled);
        keyField.setEnabled(enabled);
    }

    @Override
    public void read(IniFile ini) {
        IniSection s = ini.getSection(INI_SECTION_NAME, false);
        if ( s != null ) {
            channelField.setValue(Integer.valueOf(s.getValue(INI_CHANNEL_KEY)));
            networkIDField.setValue(Integer.valueOf(s.getValue(INI_NETWORKID_KEY)));
            nodeIDField.setValue(Integer.valueOf(s.getValue(INI_NODEID_KEY)));
            keyField.setText(s.getValue(INI_NETWORKKEY_KEY));
        }
    }

    @Override
    public void write(IniFile ini) {
        IniSection s = ini.getSection(INI_SECTION_NAME, false);
        if ( s == null ) {
            s = new IniSection(INI_SECTION_NAME);
            ini.addSection(s);
        }
        s.addKey(INI_CHANNEL_KEY, String.valueOf(channelField.getValue()));
        s.addKey(INI_NETWORKID_KEY, String.valueOf(networkIDField.getValue()));
        s.addKey(INI_NODEID_KEY, String.valueOf(nodeIDField.getValue()));
        s.addKey(INI_NETWORKKEY_KEY, keyField.getText());
    }

    protected void recalculateTextLength() {
        String text = keyField.getText();
        int len = text == null ? 0 : text.length();
        lengthField.setText(String.valueOf(len));
    }

    @Override
    public void textValueChanged(TextEvent e) {
        recalculateTextLength();
    }
}