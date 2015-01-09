package org.meshwork.app.zeroconf.l3.node.gui;

import com.prosyst.pgui.*;
import com.prosyst.pgui.border.EmptyBorder;
import com.prosyst.pgui.io.IniFile;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class AboutDialog extends PActionDialog implements ActionListener, AbstractElement {
//    final static int D_WIDTH = 250;
//    final static int D_HEIGHT = 150;
    static String CMD_OK = "CMD_OK";

    /**
     * The boolean flag used for lazy initialization.Set on false when init() is completed.
     */
    protected boolean initFlag = true;


    public AboutDialog(PFrame owner) {
        super(owner, true);
    }


    public void init(Object context) {
        initFlag = false;
        setTitle("About");
        PPanel panel = new PPanel(new BorderLayout());
        ImageIcon logoIcon = new ImageIcon(getClass().getResource("res/meshwork_64.png"));
        String text = "    L3 ZeroConf Tool    \n    Meshwork Project, 2015    \n    Author: Sinisha Djukic    ";
        PLabel label = new PLabel(text);
        label.setIcon(logoIcon);
        label.setHorizontalAlignment(PLabel.CENTER);
        panel.add(label);
        panel.setBorder(new EmptyBorder(5, 5, 5, 5));

        PButton okButton = new PButton("OK");
        okButton.setActionCommand(CMD_OK);
        okButton.addActionListener(this);
//        okButton.setPreferredSize(new Dimension(50, 30));
        PPanel okPanel = new PPanel();
        okPanel.add(okButton);
        panel.add(BorderLayout.SOUTH, okPanel);
        getRootPane().setDefaultButton(okButton);
        getContentPane().add(panel);
//        setSize(D_WIDTH, D_HEIGHT);
        pack();
    }


    /**
     * A standard method for the <subCode>ActionListener</subCode> interface.
     * It closes the frame after clicking "OK" button.
     *
     * @param e an <subCode>ActionEvent</subCode> fired on a button click.
     */
    public void actionPerformed(ActionEvent e) {
        String cmd = e.getActionCommand();
        if (cmd.equals(CMD_OK)) {
            this.setVisible(false);
        }

    }

    @Override
    public void deinit(Object context) {

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
}
