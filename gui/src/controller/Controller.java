package controller;

import data.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JComboBox;
import javax.swing.JFileChooser;
import view.EtiGP;
import view.MainFrame;

/**
 *
 * @author Philip Becker-Ehmck
 */
public class Controller {

    private MainFrame view;
    private DataInput[][] matrixList;
    private int currentMatrix; // number of the currently display matrix

    /**
     *
     * Creates an instance of a Controller object
     *
     * @param view the mainframe of the GUI
     * @param matrixList the data the GUI should correspond to
     */
    public Controller(MainFrame view) {
        this.view = view;
        this.currentMatrix = -1;

        view.addFileMenuListener(new Controller.FileMenuListener());
        view.addHelpMenuListener(new Controller.HelpMenuListener());
        view.addAccessModeListener(new Controller.AccessModeListener());
    }

    public Controller(MainFrame view, DataInput[][] matrixList) {
        this.view = view;
        this.matrixList = matrixList;
        this.currentMatrix = -1;
        addMatrices();

        view.addFileMenuListener(new Controller.FileMenuListener());
        view.addHelpMenuListener(new Controller.HelpMenuListener());
        view.addAccessModeListener(new Controller.AccessModeListener());
    }

    /**
     *
     */
    class FileMenuListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            switch (e.getActionCommand()) {
                case "open-file":
                    int returnVal = view.fileChooser.showOpenDialog(view);
                    if (returnVal == JFileChooser.APPROVE_OPTION) {
                        File file = view.fileChooser.getSelectedFile();

                        try {
                            DataInput[][] tmp = DataReader.readData(file);
                            removeMatrices();
                            matrixList = tmp;
                            new Thread(new Runnable() {

                                @Override
                                public void run() {
                                    addMatrices();
                                }
                            }).start();

                        } catch (FileNotFoundException ex) {
                            Logger.getLogger(EtiGP.class.getName()).log(Level.SEVERE, null,
                                    ex);
                        } catch (IOException ex) {
                            Logger.getLogger(EtiGP.class.getName()).log(Level.SEVERE, null,
                                    ex);
                        }
                    } else {
                        // System.out.println("File access cancelled by user.");
                    }
                    break;
                case "exit":
                    System.exit(0);
                    break;
            }
        }
    }

    /**
     *
     */
    class HelpMenuListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            switch (e.getActionCommand()) {
                case "doc":
                    break;
                case "about":
                    view.showAboutDialog();
                    break;
            }
        }
    }

    /**
     * ActionListener which checks whether a new matrix/data structre has been
     * selected in the toolbar. Initiates the update process of the GUI.
     
    class MatrixListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            displayMatrix(Integer.valueOf(e.getActionCommand()));
            view.getDropDownButton().setIcon(
                    new TextIcon(view.getDropDownButton(),
                    matrixList[Integer.valueOf(e.getActionCommand())][0].getName()));
            view.getLoadButton().setSelected(true);
        }
    }
    */
    
    /**
     * ActionListener which checks whether a new matrix/data structre has been
     * selected in the toolbar. Initiates the update process of the GUI.
     */
    class MatrixComboListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            JComboBox cb = (JComboBox)e.getSource();
            System.out.println(cb.getSelectedIndex());
            if (cb.getSelectedIndex() >= 0)
            displayMatrix(cb.getSelectedIndex());
        }
    }

    /**
     *
     */
    class AccessModeListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            // If no data is loaded, avoids null pointer exception
            if (currentMatrix < 0) {
                return;
            }
            // load = 0, store = 1
            view.removeArrowStatistics();
            view.removePieChart();
            view.removeAbsoluteRepresentation();
            switch (e.getActionCommand()) {
                case "load":
                    addArrowStatistics(matrixList[currentMatrix][0]);
                    view.addPieChart(matrixList[currentMatrix][0]);
                    view.addAbsoluteRepresentation(matrixList[currentMatrix][0]);
                    break;
                case "store":
                    addArrowStatistics(matrixList[currentMatrix][1]);
                    view.addPieChart(matrixList[currentMatrix][1]);
                    view.addAbsoluteRepresentation(matrixList[currentMatrix][1]);
                    break;
            }
        }
    }

    /**
     *
     * When a new .etis file is openend, this function initates the process to
     * add all matrices to the toolbar and to calculate the overall statistics.
     *
     * @param matrixList a complete of all matrices in the file, only overall
     * hits and misses are required as information
     */
    private void addMatrices() {

        // Variables for the overall statistics panel
        long overallHits = 0;
        long overallMisses = 0;
        List<String> names = new LinkedList();


        // Go through the list of matrices and calculate overall statistics
        for (DataInput[] matrix : matrixList) {
            names.add(matrix[0].getName());
            for (DataInput m : matrix) {
                int[] numAcesses = m.getAbsoluteNumAccesses();
                overallHits += numAcesses[0];
                overallMisses += numAcesses[1];
            }
        }
        // Add general statistics and the dropdown menu
        view.updateDropdownMenu(names);
        view.updateOverallStatistics(overallHits, overallMisses);
        // Automatically display data from the first matrix
        displayMatrix(0);
        currentMatrix = 0;
        
        view.addMatrixComboListener(new Controller.MatrixComboListener());
    }

    /**
     *
     * Clears the memory of the old dataset when a new .etis file is loaded.
     */
    private void removeMatrices() {
        matrixList = null;
        currentMatrix = -1;
    }

    /**
     *
     * Updates the GUI to display all relevant data to the specified matrix n.
     * That includes the absolute matrix representation, the relative jumps
     * (arrows) and the pie chart diagram.
     *
     * @param n , number of the selected matrix
     */
    private void displayMatrix(int n) {
        // Removes obsolete data
        DataInput[] matrix = matrixList[n];
        view.removeAbsoluteRepresentation();
        view.removeArrowStatistics();
        view.removePieChart();
        view.removeSequences();
        view.removePatterns();
        
        // Add/Update relevant data
        currentMatrix = n;
        updateMatrixStatistics(matrix);
        view.addAbsoluteRepresentation(matrix[0]);
        addArrowStatistics(matrix[0]);
        view.addPieChart(matrix[0]);
        matrix[0].getSequences();
        addSequences(matrix[0]);
        addPatterns(matrix[0]);
        view.getLoadButton().setSelected(true);
    }
    
    private void updateMatrixStatistics(DataInput[] matrix) {
        int[] numLoadAccesses = matrix[0].getAbsoluteNumAccesses();
        int[] numStoreAccesses = matrix[1].getAbsoluteNumAccesses();
        
        view.updateMatrixStatistics(numLoadAccesses, numStoreAccesses, matrix[0].getName());
    }

    /**
     *
     * @param matrix
     */
    private void addSequences(DataInput matrix) {
        Sequence[] sequences = matrix.getSequences();
        for (Sequence seq : sequences) {
            view.addSequences(seq);
        }
    }
    
    private void addPatterns(DataInput matrix) {
        Pattern[] patterns = matrix.getPatterns();
        for (Pattern p : patterns) {
            view.addPattern(p);
        }
        view.getPatternsPanel().fillSpace();
    }

    /**
     *
     * Creates the ArrowStatisticsPanels for the specified matrix
     *
     * @param matrix
     */
    private void addArrowStatistics(DataInput matrix) {
        List<RelativeJump> list = matrix.getRelativeJumps();

        for (RelativeJump jump : list) {
            view.addArrowStatistics(jump);
        }
        view.getRelativeJumpPanel().fillSpace();
    }
}