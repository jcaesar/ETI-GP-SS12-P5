/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package view;

import controller.Controller;
import data.*;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Random;
import java.util.logging.Level;
import java.util.logging.Logger;

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

        /**
        DataInput matricesList = new StandardInput();
        DataInput[] tmp;
        try {
            tmp = DataReader.readData(new File("F:\\Downloads\\mm.etis"));
            matrixList.addAll(Arrays.asList(tmp));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(EtiGP.class.getName()).log(Level.SEVERE, null, ex);
        } catch (IOException ex) {
            Logger.getLogger(EtiGP.class.getName()).log(Level.SEVERE, null, ex);
        }
        
        */
        // Create frame and controller instance
        MainFrame frame = new MainFrame();
        Controller c = new Controller(frame, matrixList);
        
        // Display frame
        frame.setVisible(true);
        
        // Add matrices to the frame
        // c.addMatrices(matrixList);
    }
}
