package org.meshwork.app.zeroconf.l3.node.gui;

import com.prosyst.pgui.*;
import com.prosyst.pgui.io.IniFile;
import com.prosyst.pgui.io.IniSection;
import com.prosyst.pgui.layout.GridBagConstraints2;
import org.meshwork.core.zeroconf.l3.Constants;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public class ReportingPanel extends PTitledPanel implements AbstractElement, ActionListener {

    public static final String INI_SECTION_NAME = "Reporting";
    public static final String INI_FLAGS_KEY = "Flags";
    public static final String INI_NODEID_KEY = "Node ID";

    protected PScrollNumberField nodeIDField;
    protected PTextField maskField;
    protected PCheckBox checkNetworkAddRemove;
    protected PCheckBox checkDiscreteChange;
    protected PCheckBox checkThresholdChange;
    protected boolean ignoreUpdate = false;

    public ReportingPanel() {
        super("Reporting");
    }

    @Override
    public void init(Object context) {
        containerPanel.setLayout(new GridBagLayout());

        //Node ID
        containerPanel.add(new PLabel("Node ID:"), new GridBagConstraints2(0, 0, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 0), 0, 3));
        containerPanel.add(nodeIDField = new PScrollNumberField(), new GridBagConstraints2(1, 0, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 3), 0, 3));
        nodeIDField.setMinimum(Constants.MIN_NODE_ID);
        nodeIDField.setMaximum(Constants.MAX_NODE_ID);

        //Mask
        containerPanel.add(new PLabel("Mask:"), new GridBagConstraints2(0, 1, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 0), 0, 3));
        containerPanel.add(maskField = new PTextField(), new GridBagConstraints2(1, 1, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 3), 0, 3));
        maskField.setEditable(false);

        //Checkboxes
        containerPanel.add(checkNetworkAddRemove = new PCheckBox("Network Add/Remove (0x" + Integer.toHexString(Constants.MASK_REPORT_NWK_ADD_REMOVE) + ")"), new GridBagConstraints2(0, 2, 2, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 0), 0, 3));
        containerPanel.add(checkDiscreteChange = new PCheckBox("Discrete Change (0x" + Integer.toHexString(Constants.MASK_REPORT_DISCRETE_CHANGE) + ")"), new GridBagConstraints2(0, 3, 2, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 0), 0, 3));
        containerPanel.add(checkThresholdChange = new PCheckBox("Threshold Change (0x" + Integer.toHexString(Constants.MASK_REPORT_THRESHOLD_CHANGE) + ")"), new GridBagConstraints2(0, 4, 2, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 0), 0, 3));
        checkNetworkAddRemove.addActionListener(this);
        checkDiscreteChange.addActionListener(this);
        checkThresholdChange.addActionListener(this);

        updateMaskField();
    }

    @Override
    public void deinit(Object context) {
        checkNetworkAddRemove.removeActionListener(this);
        checkDiscreteChange.removeActionListener(this);
        checkThresholdChange.removeActionListener(this);
        containerPanel.removeAll();
    }

    @Override
    public Object getData() {
        ReportingData data = new ReportingData();
        data.reportingFlags = getMask();
        data.reportingNodeID = (byte) getNodeID();
        return data;
    }

    public byte getNodeID() {
        return (byte) nodeIDField.getValue();
    }

    public void setNodeID(byte value) {
        nodeIDField.setValue((char) value);
    }

    @Override
    public void setData(Object data) {
        if ( data == null ) {
            setMask((byte)0);
            setNodeID((byte) 0);
        } else if ( data instanceof ReportingData) {
            setMask(((ReportingData) data).reportingFlags);
            setNodeID(((ReportingData) data).reportingNodeID);
        }
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        Object src = e.getSource();
        if ( !ignoreUpdate ) {
            if (src == checkNetworkAddRemove) {
                updateMaskField();
            } else if (src == checkDiscreteChange) {
                updateMaskField();
            } else if (src == checkThresholdChange) {
                updateMaskField();
            }
        }
    }

    public byte getMask() {
        return (byte)  ((checkNetworkAddRemove.isSelected() ? Constants.MASK_REPORT_NWK_ADD_REMOVE : 0) |
                        (checkDiscreteChange.isSelected() ? Constants.MASK_REPORT_DISCRETE_CHANGE : 0) |
                        (checkThresholdChange.isSelected() ? Constants.MASK_REPORT_THRESHOLD_CHANGE : 0));
    }

    public void setMask(byte mask) {
        ignoreUpdate = true;
        checkNetworkAddRemove.setSelected((mask & Constants.MASK_REPORT_NWK_ADD_REMOVE) != 0);
        checkDiscreteChange.setSelected((mask & Constants.MASK_REPORT_DISCRETE_CHANGE) != 0);
        checkThresholdChange.setSelected((mask & Constants.MASK_REPORT_THRESHOLD_CHANGE) != 0);
        ignoreUpdate = false;
        updateMaskField();
    }

    public void updateMaskField() {
        String value = "" + getMask();
        maskField.setText(value);
    }

    @Override
    public void setEnabled(boolean enabled) {
        maskField.setEnabled(enabled);
        nodeIDField.setEnabled(enabled);
        checkNetworkAddRemove.setEnabled(enabled);
        checkDiscreteChange.setEnabled(enabled);
        checkThresholdChange.setEnabled(enabled);
    }

    @Override
    public void read(IniFile ini) {
        IniSection s = ini.getSection(INI_SECTION_NAME, false);
        if ( s != null ) {
            String v1 = s.getValue(INI_FLAGS_KEY);
            String v2 = s.getValue(INI_NODEID_KEY);
            setMask(Integer.valueOf(v1).byteValue());
            setNodeID(Integer.valueOf(v2).byteValue());
        }
    }

    @Override
    public void write(IniFile ini) {
        IniSection s = ini.getSection(INI_SECTION_NAME, false);
        if ( s == null ) {
            s = new IniSection(INI_SECTION_NAME);
            ini.addSection(s);
        }
        s.addKey(INI_FLAGS_KEY, String.valueOf(getMask()));
        s.addKey(INI_NODEID_KEY, String.valueOf(getNodeID()));
    }
}
