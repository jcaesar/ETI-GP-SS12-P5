/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package view;

import controller.Controller;
import data.*;
import java.util.ArrayList;
import java.util.Random;

/**
 *
 * @author Philip Becker-Ehmck
 */
public class EtiGP {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) throws InterruptedException {

        final ArrayList<DataInput> matrixList = new ArrayList<DataInput>();

        // Generate 1-5 matrices and fill them with dummy data
        Random rand = new Random();
        int max = rand.nextInt(10) + 5;

        for (int i = 0; i < max; i++) {
            matrixList.add(new DummyInput());
        }
        
        // Create frame and controller instance
        MainFrame frame = new MainFrame(matrixList);
        Controller c = new Controller(frame, matrixList);
        
        // Display frame
        frame.setVisible(true);
        
        // Add matrices to the frame
        c.addMatrices(matrixList);
    }
}
