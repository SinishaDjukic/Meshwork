package org.meshwork.app.zeroconf.l3.node.gui;

import com.prosyst.pgui.*;
import com.prosyst.pgui.border.EmptyBorder;
import com.prosyst.pgui.io.IniFile;
import com.prosyst.pgui.io.IniSection;
import com.prosyst.pgui.layout.VerticalFlowLayout;
import org.meshwork.core.host.l3.Constants;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public class DeliveryPanel extends PTitledPanel implements AbstractElement, ActionListener {

    public static final String INI_SECTION_NAME = "Delivery";
    public static final String INI_DELIVERY_KEY = "Mask";

    protected PTextField maskField;
    protected PCheckBox checkDirect;
    protected PCheckBox checkRouted;
    protected PCheckBox checkFlood;
    protected boolean ignoreUpdate = false;

    public DeliveryPanel() {
        super("Delivery");
    }

    @Override
    public void init(Object context) {
        containerPanel.setLayout(new VerticalFlowLayout());

        //Mask
        BorderLayout layout = new BorderLayout();
        layout.setHgap(3);
        PPanel pmask = new PPanel(layout);
        pmask.setBorder(new EmptyBorder(0, 2, 0, 2));
        pmask.add(new PLabel("Mask:"), BorderLayout.WEST);
        pmask.add(maskField = new PTextField(), BorderLayout.CENTER);
        maskField.setEditable(false);
        containerPanel.add(pmask);

        //Checkboxes
        containerPanel.add(checkDirect = new PCheckBox("Direct (0x" + Integer.toHexString(Constants.DELIVERY_DIRECT) + ")"));
        containerPanel.add(checkRouted = new PCheckBox("Routed (0x" + Integer.toHexString(Constants.DELIVERY_ROUTED) + ")"));
        containerPanel.add(checkFlood = new PCheckBox("Flood (0x" + Integer.toHexString(Constants.DELIVERY_FLOOD) + ")"));

        checkDirect.addActionListener(this);
        checkRouted.addActionListener(this);
        checkFlood.addActionListener(this);

        updateMaskField();
    }

    @Override
    public void deinit(Object context) {
        checkDirect.removeActionListener(this);
        checkRouted.removeActionListener(this);
        checkFlood.removeActionListener(this);
        containerPanel.removeAll();
    }

    @Override
    public Object getData() {
        DeliveryPanelData data = new DeliveryPanelData();
        data.delivery = getMask();
        return data;
    }

    @Override
    public void setData(Object data) {
        if ( data == null ) {
            setMask((byte)0);
        } else if ( data instanceof DeliveryPanelData ) {
            setMask(((DeliveryPanelData) data).delivery);
        } else if ( data instanceof Byte ) {
            setMask((Byte) data);
        }
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        Object src = e.getSource();
        if ( !ignoreUpdate ) {
            if (src == checkDirect) {
                updateMaskField();
            } else if (src == checkRouted) {
                updateMaskField();
            } else if (src == checkFlood) {
                updateMaskField();
            }
        }
    }

    public byte getMask() {
        return (byte)  ((checkDirect.isSelected() ? Constants.DELIVERY_DIRECT : 0) |
                        (checkRouted.isSelected() ? Constants.DELIVERY_ROUTED : 0) |
                        (checkFlood.isSelected() ? Constants.DELIVERY_FLOOD : 0));
    }

    public void setMask(byte mask) {
        ignoreUpdate = true;
        checkDirect.setSelected((mask & Constants.DELIVERY_DIRECT) != 0);
        checkRouted.setSelected((mask & Constants.DELIVERY_ROUTED) != 0);
        checkFlood.setSelected((mask & Constants.DELIVERY_FLOOD) != 0);
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
        checkDirect.setEnabled(enabled);
        checkRouted.setEnabled(enabled);
        checkFlood.setEnabled(enabled);
    }

    @Override
    public void read(IniFile ini) {
        IniSection s = ini.getSection(INI_SECTION_NAME, false);
        if ( s != null ) {
            String v = s.getValue(INI_DELIVERY_KEY);
            setMask(Integer.valueOf(v).byteValue());
        }
    }

    @Override
    public void write(IniFile ini) {
        IniSection s = ini.getSection(INI_SECTION_NAME, false);
        if ( s == null ) {
            s = new IniSection(INI_SECTION_NAME);
            ini.addSection(s);
        }
        s.addKey(INI_DELIVERY_KEY, String.valueOf(getMask()));
    }
}
