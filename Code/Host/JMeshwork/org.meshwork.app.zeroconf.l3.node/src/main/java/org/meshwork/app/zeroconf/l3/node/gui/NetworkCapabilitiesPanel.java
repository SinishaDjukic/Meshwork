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
public class NetworkCapabilitiesPanel extends PTitledPanel implements AbstractElement, ActionListener {

    public static final String INI_SECTION_NAME = "Network Capabilities";
    public static final String INI_CAPABILITIES_KEY = "Capabilities";

    protected PTextField maskField;
    protected ButtonGroup wakeupGroup;
    protected PRadioButton checkAlwaysListening;
    protected PRadioButton checkPeriodicWakeup;
    protected PRadioButton checkAlwaysSleeping;
    protected PCheckBox checkRouter;
    protected PCheckBox checkEdge;
    protected boolean ignoreActionEvents = false;

    public NetworkCapabilitiesPanel() {
        super("Network Capabilities");
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
        containerPanel.add(checkAlwaysListening = new PRadioButton("Always Listening (0x" + Integer.toHexString(Constants.NWKCAPS_ALWAYS_LISTENING) + ")"));
        containerPanel.add(checkPeriodicWakeup = new PRadioButton("Periodic Wakeup (0x" + Integer.toHexString(Constants.NWKCAPS_PERIODIC_WAKEUP) + ")"));
        containerPanel.add(checkAlwaysSleeping = new PRadioButton("Always Sleeping (0x" + Integer.toHexString(Constants.NWKCAPS_ALWAYS_SLEEPING) + ")"));
        containerPanel.add(checkRouter = new PCheckBox("Router (0x" + Integer.toHexString(Constants.NWKCAPS_ROUTER) + ")"));
        containerPanel.add(checkEdge = new PCheckBox("Edge (0x" + Integer.toHexString(Constants.NWKCAPS_EDGE) + ")"));

        wakeupGroup = new ButtonGroup();
        wakeupGroup.add(checkAlwaysListening);
        wakeupGroup.add(checkPeriodicWakeup);
        wakeupGroup.add(checkAlwaysSleeping);

        checkAlwaysListening.addActionListener(this);
        checkPeriodicWakeup.addActionListener(this);
        checkAlwaysSleeping.addActionListener(this);
        checkRouter.addActionListener(this);
        checkEdge.addActionListener(this);

        updateMaskField();
    }

    @Override
    public void deinit(Object context) {
        checkAlwaysListening.removeActionListener(this);
        checkPeriodicWakeup.removeActionListener(this);
        checkAlwaysSleeping.removeActionListener(this);
        checkRouter.removeActionListener(this);
        checkEdge.removeActionListener(this);
        containerPanel.removeAll();
    }

    @Override
    public Object getData() {
        NetworkCapabilitiesData data = new NetworkCapabilitiesData();
        data.capabilities = getMask();
        return data;
    }

    @Override
    public void setData(Object data) {
        if ( data == null ) {
            setMask((byte)0);
        } else if ( data instanceof NetworkCapabilitiesData) {
            setMask(((NetworkCapabilitiesData) data).capabilities);
        } else if ( data instanceof Byte ) {
            setMask((Byte) data);
        }
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        Object src = e.getSource();
        if ( !ignoreActionEvents ) {
            if (src == checkAlwaysListening) {
                updateMaskField();
            } else if (src == checkPeriodicWakeup) {
                updateMaskField();
            } else if (src == checkAlwaysSleeping) {
                updateMaskField();
            } else if (src == checkRouter) {
                updateMaskField();
            } else if (src == checkEdge) {
                updateMaskField();
            }
        }
    }

    public byte getMask() {
        return (byte)  ((checkAlwaysListening.isSelected() ? Constants.NWKCAPS_ALWAYS_LISTENING: 0) |
                (checkPeriodicWakeup.isSelected() ? Constants.NWKCAPS_PERIODIC_WAKEUP : 0) |
                (checkAlwaysSleeping.isSelected() ? Constants.NWKCAPS_ALWAYS_SLEEPING : 0) |
                (checkRouter.isSelected() ? Constants.NWKCAPS_ROUTER : 0) |
                (checkEdge.isSelected() ? Constants.NWKCAPS_EDGE : 0));
    }

    public void setMask(byte mask) {
        ignoreActionEvents = true;
        wakeupGroup.setSelected(wakeupGroup.getSelection(), false);
        checkAlwaysListening.setSelected((mask & 0x03) == Constants.NWKCAPS_ALWAYS_LISTENING);
        checkPeriodicWakeup.setSelected((mask & 0x03) == Constants.NWKCAPS_PERIODIC_WAKEUP);
        checkAlwaysSleeping.setSelected((mask & 0x03) == Constants.NWKCAPS_ALWAYS_SLEEPING);
        checkRouter.setSelected((mask & Constants.NWKCAPS_ROUTER) != 0);
        checkEdge.setSelected((mask & Constants.NWKCAPS_EDGE) != 0);
        ignoreActionEvents = false;
        updateMaskField();
    }

    public void updateMaskField() {
        String value = "" + getMask();
        maskField.setText(value);
    }

    @Override
    public void setEnabled(boolean enabled) {
        maskField.setEnabled(enabled);
        checkAlwaysListening.setEnabled(enabled);
        checkPeriodicWakeup.setEnabled(enabled);
        checkAlwaysSleeping.setEnabled(enabled);
        checkRouter.setEnabled(enabled);
        checkEdge.setEnabled(enabled);
    }

    @Override
    public void read(IniFile ini) {
        IniSection s = ini.getSection(INI_SECTION_NAME, false);
        if ( s != null ) {
            String v = s.getValue(INI_CAPABILITIES_KEY);
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
        s.addKey(INI_CAPABILITIES_KEY, String.valueOf(getMask()));
    }
}
