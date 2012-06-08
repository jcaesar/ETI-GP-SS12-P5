
package controller;

import data.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import view.MainFrame;
import java.util.List;

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
    }
    
    /**
     * ActionListener which checks whether a new matrix/data structre
     * has been selected in the toolbar. Initiates the update process
     * of the GUI.
     */
    class MatrixListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            displayMatrix(Integer.valueOf(e.getActionCommand().trim())-1);
        }
    }
    
    /**
     * 
     * When a new .etis file is openend, this function initates the process to
     * add all matrices to the toolbar and to calculate the overall statistics.
     * 
     * @param matrixList a complete of all matrices in the file, 
     * only overall hits and misses are required as information
     */
    public void addMatrices(ArrayList<DataInput> matrixList) {

        // Variables for the overall statistics panel
        long overallHits = 0;
        long overallMisses = 0;

        // Go through the list of matrices, add a button for every item 
        for (DataInput matrix : matrixList) {
            overallHits += matrix.getHits();
            overallMisses += matrix.getMisses();
            number++;
            view.addToggleButton(matrix, number);
        }
        // If there is more than 1 matrix to be analyzed, display an overall statistic.
        if (number > 1) {
            view.addOverallStatistics(overallHits, overallMisses);
        }
        // Automatically display data from the first matrix
        displayMatrix(0);

        // Adds ActionListeners to the buttons
        view.addMatrixListener(new MatrixListener());
    }
    
    /**
     * 
     * Clears the memory of the old dataset when a new .etis file is loaded.
     */
    public void removeMatrices() {
        throw new UnsupportedOperationException("Not yet implemented");
    }
    
    /**
     * 
     * Updates the GUI to display all relevant data to the specified matrix n.
     * That includes the absolute matrix representation, the relative jumps (arrows)
     * and the pie chart diagramm.
     * 
     * @param n , number of the selected matrix
     */
    private void displayMatrix(int n) {
        // Removes obsolete data
        DataInput matrix = matrixList.get(n);
        view.removeDetailedStatistics();
        view.removeAbsoluteRepresentation();
        view.removeArrowStatistics();
        view.removePieChart();
        // Add relevant data
        view.addDetailedStatistics(matrix, n);
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
