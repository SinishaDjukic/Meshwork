package org.meshwork.app.zeroconf.l3.node.gui;

import com.prosyst.pgui.PLabel;
import com.prosyst.pgui.PPanel;
import com.prosyst.pgui.PTextField;
import com.prosyst.pgui.PTitledPanel;
import com.prosyst.pgui.io.IniFile;
import com.prosyst.pgui.io.IniSection;
import com.prosyst.pgui.layout.GridBagConstraints2;
import com.prosyst.pgui.layout.VerticalFlowLayout;

import java.awt.*;
import java.awt.event.TextEvent;
import java.awt.event.TextListener;

/**
 * Created by Sinisha Djukic on 31.12.2014.
 */
public class SerialNumberPanel extends PTitledPanel implements AbstractElement, TextListener {

    public static final String INI_SECTION_NAME = "Serial Number";
    public static final String INI_NUMBER_KEY = "Number";

    protected PTextField lengthField;
    protected PTextField valueField;

    public SerialNumberPanel() {
        super("Serial Number");
    }

    public void init2(Object context) {
        containerPanel.setLayout(new VerticalFlowLayout());
        //Length
        PPanel plen = new PPanel();
        plen.add(new PLabel("Length:"), BorderLayout.WEST);
        plen.add(lengthField = new PTextField(), BorderLayout.CENTER);
        lengthField.setEditable(false);
        containerPanel.add(plen);

        //Value
        PPanel pval = new PPanel();
        pval.add(new PLabel("Value:"), BorderLayout.WEST);
        pval.add(valueField = new PTextField(), BorderLayout.CENTER);
        containerPanel.add(pval);

        valueField.addTextListener(this);
        recalculateTextLength();
    }

    @Override
    public void init(Object context) {
        containerPanel.setLayout(new GridBagLayout());

        //Length
        containerPanel.add(new PLabel("Length:"), new GridBagConstraints2(0, 0, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 0), 0, 3));
        containerPanel.add(lengthField = new PTextField(), new GridBagConstraints2(1, 0, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 3), 0, 3));
        lengthField.setEditable(false);

        //Value
        containerPanel.add(new PLabel("Value:"), new GridBagConstraints2(0, 1, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 0), 0, 3));
        containerPanel.add(valueField = new PTextField(), new GridBagConstraints2(1, 1, 1, 1, 1, 1, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(3, 3, 3, 3), 0, 3));

        valueField.addTextListener(this);
        recalculateTextLength();
    }

    @Override
    public void deinit(Object context) {
        valueField.removeTextListener(this);
        containerPanel.removeAll();
    }

    @Override
    public Object getData() {
        SerialNumberData data = new SerialNumberData();
        data.value = getValue();
        return data;
    }

    @Override
    public void setData(Object data) {
        if ( data == null ) {
            setValue("");
        } else if ( data instanceof SerialNumberData) {
            setValue(((SerialNumberData) data).value);
        } else if ( data instanceof String ) {
            setValue((String) data);
        }
    }

    public String getValue() {
        return valueField.getText();
    }

    public void setValue(String value) {
        valueField.setText(value);
    }

    @Override
    public void setEnabled(boolean enabled) {
        lengthField.setEnabled(enabled);
        valueField.setEnabled(enabled);
    }

    @Override
    public void read(IniFile ini) {
        IniSection s = ini.getSection(INI_SECTION_NAME, false);
        if ( s != null ) {
            String v = s.getValue(INI_NUMBER_KEY);
            setValue(v);
        }
    }

    @Override
    public void write(IniFile ini) {
        IniSection s = ini.getSection(INI_SECTION_NAME, false);
        if ( s == null ) {
            s = new IniSection(INI_SECTION_NAME);
            ini.addSection(s);
        }
        s.addKey(INI_NUMBER_KEY, getValue());
    }

    protected void recalculateTextLength() {
        String text = valueField.getText();
        int len = text == null ? 0 : text.length();
        lengthField.setText(String.valueOf(len));
    }

    @Override
    public void textValueChanged(TextEvent e) {
        recalculateTextLength();
    }
}