/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package view;

import controller.Controller;

/**
 *
 * @author Philip Becker-Ehmck
 */
public class EtiGP {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        
        java.awt.EventQueue.invokeLater(new Runnable() {

            @Override
            public void run() {
                // new MainFrame().setVisible(true);
                final MainFrame frame = new MainFrame();
                Controller c = new Controller(frame);
                frame.setVisible(true);
            }
        });
    }
}
