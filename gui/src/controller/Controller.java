package controller;

import data.DataInput;
import data.DataReader;
import data.RelativeJump;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JFileChooser;
import view.EtiGP;
import view.MainFrame;

/**
 *
 * @author Philip Becker-Ehmck
 */
public class Controller {

    private MainFrame view;
    private ArrayList<DataInput> matrixList;
    private int number;

    /**
     *
     * Creates an instance of an Controller object
     *
     * @param view the mainframe of the GUI
     * @param matrixList the data the GUI should correspond to
     */
    public Controller(MainFrame view, ArrayList<DataInput> matrixList) {
        this.view = view;
        this.matrixList = matrixList;
        number = 0;

        view.addFileMenuListener(new FileMenuListener());
        view.addHelpMenuListener(new HelpMenuListener());
    }

    /**
     * ActionListener which checks whether a new matrix/data structre has been
     * selected in the toolbar. Initiates the update process of the GUI.
     */
    class MatrixListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            displayMatrix(Integer.valueOf(e.getActionCommand()) - 1);
        }
    }

    /**
     *
     */
    class FileMenuListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {

            if (e.getSource() == view.openFileMenuItem) {
                int returnVal = view.fileChooser.showOpenDialog(view);
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    File file = view.fileChooser.getSelectedFile();

                    DataInput[] tmp;
                    try {
                        tmp = DataReader.readData(file);
                        removeMatrices();
                        matrixList.addAll(Arrays.asList(tmp));
                        addMatrices(matrixList);
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
            } else if (e.getSource() == view.exitMenuItem) {
                System.exit(0);
            }
        }
    }

    class HelpMenuListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            if (e.getSource() == view.docMenuItem) {
                // TODO
            } else if (e.getSource() == view.aboutMenuItem) {
                view.showAboutDialog();
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
    private void addMatrices(ArrayList<DataInput> matrixList) {

        // Variables for the overall statistics panel
        long overallHits = 0;
        long overallMisses = 0;

        // Go through the list of matrices, add a button for every item 
        for (DataInput matrix : matrixList) {
            long[] numAcesses = matrix.getAbsoluteNumAccesses();
            overallHits += numAcesses[0];
            overallMisses += numAcesses[1];
            number++;
            view.addToggleButton(matrix, number);
        }

        view.updateOverallStatistics(overallHits, overallMisses);
        // Automatically display data from the first matrix
        displayMatrix(0);

        // Adds ActionListeners to the buttons
        view.addMatrixListener(new MatrixListener());
    }

    /**
     *
     * Clears the memory of the old dataset when a new .etis file is loaded.
     */
    private void removeMatrices() {
        view.removeToggleButton();
        matrixList = new ArrayList<DataInput>();
        number = 0;
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
        DataInput matrix = matrixList.get(n);
        view.removeAbsoluteRepresentation();
        view.removeArrowStatistics();
        view.removePieChart();
        // Add relevant data
        long[] numAcesses = matrix.getAbsoluteNumAccesses();
        view.updateDetailedStatistics(numAcesses[0], numAcesses[1], n);
        view.addAbsoluteRepresentation(matrix);
        addArrowStatistics(matrix);
        view.addPieChart(matrix);
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
            view.addArrowStatisticsPanel(jump);
        }
    }
}
