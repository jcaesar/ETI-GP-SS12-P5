package view;

import controller.EtisFileFilter;
import data.DataInput;
import data.Pattern;
import data.RelativeJump;
import data.Sequence;
import java.awt.Component;
import java.awt.GridBagConstraints;
import java.awt.event.ActionListener;
import java.util.List;
import javax.swing.*;

/**
 *
 * @author Philip Becker-Ehmck
 */
public class MainFrame extends javax.swing.JFrame {

    private PatternDetailsPanel detailedPanel;

    /**
     * Creates new MainFrame
     */
    public MainFrame() {
        initComponents();
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        dataStructuresButtonGroup = new javax.swing.ButtonGroup();
        fileChooser = new javax.swing.JFileChooser();
        aboutDialog = new javax.swing.JDialog();
        aboutPanel = new javax.swing.JPanel();
        aboutScrollPane = new javax.swing.JScrollPane();
        aboutTextArea = new javax.swing.JTextArea();
        teamScrollPane = new javax.swing.JScrollPane();
        teamTextArea = new javax.swing.JTextArea();
        loadStoreButtonGroup = new javax.swing.ButtonGroup();
        helpDialog = new javax.swing.JDialog();
        helpPanel = new javax.swing.JPanel();
        helpScrollPane = new javax.swing.JScrollPane();
        helpPane = new javax.swing.JEditorPane();
        toolbar = new javax.swing.JToolBar();
        labelToolBar = new javax.swing.JLabel();
        dropdownMenu = new javax.swing.JComboBox();
        filler2 = new javax.swing.Box.Filler(new java.awt.Dimension(0, 0), new java.awt.Dimension(0, 0), new java.awt.Dimension(32767, 0));
        filler3 = new javax.swing.Box.Filler(new java.awt.Dimension(0, 0), new java.awt.Dimension(0, 0), new java.awt.Dimension(32767, 0));
        loadButton = new javax.swing.JToggleButton();
        storeButton = new javax.swing.JToggleButton();
        relativeAccessesTab = new javax.swing.JTabbedPane();
        relativeJumpsTab = new javax.swing.JPanel();
        relativeAccessPanel = new view.RelativeAccessPanel();
        piePanel = new javax.swing.JPanel();
        patternsTab = new javax.swing.JPanel();
        patternsOverviewPanel = new view.PatternOverviewPanel();
        patternDetailsPanel = new javax.swing.JPanel();
        matrixPanel = new javax.swing.JPanel();
        filler1 = new javax.swing.Box.Filler(new java.awt.Dimension(0, 10), new java.awt.Dimension(0, 10), new java.awt.Dimension(32767, 5));
        statisticsPanel = new view.StatisticsPanel();
        menuBar = new javax.swing.JMenuBar();
        fileMenu = new javax.swing.JMenu();
        openFileMenuItem = new javax.swing.JMenuItem();
        seperator = new javax.swing.JPopupMenu.Separator();
        exitMenuItem = new javax.swing.JMenuItem();
        helpMenu = new javax.swing.JMenu();
        docMenuItem = new javax.swing.JMenuItem();
        aboutMenuItem = new javax.swing.JMenuItem();

        fileChooser.setFileFilter(new EtisFileFilter());

        aboutDialog.setTitle("About ETIS");
        aboutDialog.setResizable(false);

        aboutPanel.setMinimumSize(new java.awt.Dimension(450, 300));
        aboutPanel.setPreferredSize(new java.awt.Dimension(450, 300));
        aboutPanel.setLayout(new java.awt.GridBagLayout());

        aboutScrollPane.setBorder(null);
        aboutScrollPane.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        aboutScrollPane.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_NEVER);

        aboutTextArea.setColumns(20);
        aboutTextArea.setEditable(false);
        aboutTextArea.setFont(new java.awt.Font("Monospaced", 0, 12)); // NOI18N
        aboutTextArea.setLineWrap(true);
        aboutTextArea.setRows(3);
        aboutTextArea.setTabSize(4);
        aboutTextArea.setText("ETIS is an analyzing tool that allows you to evaluate the cache performance of matrix operations.");
        aboutTextArea.setWrapStyleWord(true);
        aboutTextArea.setMinimumSize(new java.awt.Dimension(450, 300));
        aboutTextArea.setName("");
        aboutTextArea.setOpaque(false);
        aboutScrollPane.setViewportView(aboutTextArea);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.ipadx = 407;
        gridBagConstraints.ipady = 46;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(11, 10, 0, 10);
        aboutPanel.add(aboutScrollPane, gridBagConstraints);

        teamTextArea.setColumns(20);
        teamTextArea.setEditable(false);
        teamTextArea.setFont(new java.awt.Font("Monospaced", 0, 12)); // NOI18N
        teamTextArea.setRows(5);
        teamTextArea.setText("Developers\n------\nPhilip Becker-Ehmck\nThomas Breier\nMatthias Brugger\nJakob Buchgraber\nDominik Durner\nNils Kunze\nJulius Michaelis\nSimon Wimmer\n");
        teamScrollPane.setViewportView(teamTextArea);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.ipadx = 407;
        gridBagConstraints.ipady = 180;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(6, 10, 11, 10);
        aboutPanel.add(teamScrollPane, gridBagConstraints);

        aboutDialog.getContentPane().add(aboutPanel, java.awt.BorderLayout.LINE_START);

        helpDialog.setTitle("Help Contents");

        helpPanel.setLayout(new java.awt.GridLayout(1, 0));

        helpPane.setContentType("text/html");
        helpPane.setEditable(false);
        helpPane.setText("<html>\r\n  <head>\r\n\r\n  </head>\r\n  <body>\r\n    <p style=\"margin-top: 0\">\r\n     \n<b>\rData Structures</b><br /> \nIn the selector right next to this label, one can select a matrix by its name, for which all of the information gathered by McTracer should be displayed.\n\n<br /> <br /> \n<b>Load/Store switch</b><br /> \nThis switch is located in the top right corner and determines if the information displayed throughout the rest of the GUI refers to load or store type accesses.\n\n<br /> <br /> \n<b>Absolute Matrix Representation</b><br /> \nThis part visualizes the accesses (that is loads or stores as selected by the load/store switch) that happened on the matrix on a per field basis. Each field is coloured on a scale from red to green, where red means that 0% of the accesses on this field were hits, whereas green means that 100% of the accesses were successful. This colour scheme is also used throughout the rest of the graphics appearing in the GUI.\n\n<br /> <br /> \n<b>Relative Accesses</b><br /> In this part the single accesses (that is loads or stores as selected by the load/store switch) on different fields are classified by the difference in position of two consecutive accesses. For example, if the program makes two consecutive accesses to the fields at positions (1|1) and (1|3) this will give a new relative access class identified by the position delta of (0|2). The following information is displayed for the 8 access classes that were found most often: The number of hits and misses that occurred for accesses belonging to that class, the corresponding hit/miss ratio and a graphical representation of the access' position delta with an arrow. These arrows are coloured in accordance to the hit/miss ratio.\nIn the area the hit/miss ratio for each relative access is visualized with a pie chart. The area of each pie chart corresponds to the overall number of accesses that where found for this relative access compared to the others. That means an access with a lot of occurrences will have a big chart, while an access with fewer occurrences will have a smaller pie chart. It can happen that some of the accesses do not appear as a pie chart, because their charts would be too small to be displayed.\n\n<br /> <br /> \n<b>Patterns</b><br />\nA pattern groups multiple relative accesses into a bigger sequence in order to give the user a feeling of how the matrix is traversed by the program. By selecting a pattern from the list, the following detail information is shown below the pattern list: Each pattern consists of a number of relative accesses that are traversed in the displayed order. Also for each pattern the total number of occurrences and the number of hits and misses is presented to the user. Each pattern is graphically represented by a picture showing a start point and an arrow for each consecutive relative access. Again the colouring of the arrows correlates to the hit/miss ratio. The bottom part shows a list of sequences, of which this pattern is part of. Patterns do not distinguish between load and store.\n\n<br /> <br /> \n<b>Sequences</b><br /> \nA sequence is basically a repetition of a certain pattern. It displays how often the pattern in question was repeated and which relative access did \"break\" the sequence, that means which relative access was the first one that did not belong to the pattern. For this relative access the delta in x and y position is shown. Sometimes the program can recognize a pattern that directly follows the repetition of the pattern corresponding to the sequence. If this is the case, this following pattern is identified by its id as assigned in the pattern list. Of course the same sequence can appear multiple times, so the number of occurrences of that sequence is shown. \n\n    </p>\r\n  </body>\r\n</html>\r\n");
        helpScrollPane.setViewportView(helpPane);

        helpPanel.add(helpScrollPane);

        helpDialog.getContentPane().add(helpPanel, java.awt.BorderLayout.CENTER);

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("ETIS");
        setMinimumSize(new java.awt.Dimension(1005, 700));
        setPreferredSize(new java.awt.Dimension(1005, 700));
        setResizable(false);
        getContentPane().setLayout(new java.awt.GridBagLayout());

        toolbar.setFloatable(false);
        toolbar.setRollover(true);
        toolbar.setBorderPainted(false);

        labelToolBar.setText("Data Structures: ");
        toolbar.add(labelToolBar);

        toolbar.add(dropdownMenu);
        toolbar.add(filler2);
        toolbar.add(filler3);

        loadStoreButtonGroup.add(loadButton);
        loadButton.setText("Load");
        loadButton.setFocusable(false);
        loadButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        loadButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        toolbar.add(loadButton);

        loadStoreButtonGroup.add(storeButton);
        storeButton.setText("Store");
        storeButton.setFocusable(false);
        storeButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        storeButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        toolbar.add(storeButton);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 0.1;
        getContentPane().add(toolbar, gridBagConstraints);

        relativeAccessesTab.setFocusable(false);

        relativeJumpsTab.setLayout(new java.awt.GridBagLayout());

        relativeAccessPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Relative Accesses", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Arial", 1, 14))); // NOI18N
        relativeAccessPanel.setDoubleBuffered(false);
        relativeAccessPanel.setMinimumSize(new java.awt.Dimension(500, 250));
        relativeAccessPanel.setName("");
        relativeAccessPanel.setPreferredSize(new java.awt.Dimension(500, 250));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        relativeJumpsTab.add(relativeAccessPanel, gridBagConstraints);

        piePanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Hit/Miss Ratio", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Arial", 1, 14))); // NOI18N
        piePanel.setMinimumSize(new java.awt.Dimension(500, 250));
        piePanel.setName("");
        piePanel.setPreferredSize(new java.awt.Dimension(500, 250));
        piePanel.setVerifyInputWhenFocusTarget(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        relativeJumpsTab.add(piePanel, gridBagConstraints);
        piePanel.getAccessibleContext().setAccessibleDescription("");

        relativeAccessesTab.addTab("Relative Accesses", relativeJumpsTab);

        patternsTab.setLayout(new java.awt.GridBagLayout());

        patternsOverviewPanel.setMinimumSize(new java.awt.Dimension(500, 120));
        patternsOverviewPanel.setPreferredSize(new java.awt.Dimension(500, 120));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.ipadx = 1;
        gridBagConstraints.ipady = 1;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        patternsTab.add(patternsOverviewPanel, gridBagConstraints);

        patternDetailsPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Details", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Arial", 1, 14))); // NOI18N
        patternDetailsPanel.setMinimumSize(new java.awt.Dimension(500, 250));
        patternDetailsPanel.setPreferredSize(new java.awt.Dimension(500, 250));
        patternDetailsPanel.setLayout(new java.awt.GridBagLayout());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 3.0;
        patternsTab.add(patternDetailsPanel, gridBagConstraints);

        relativeAccessesTab.addTab("Patterns", patternsTab);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.gridheight = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        getContentPane().add(relativeAccessesTab, gridBagConstraints);
        relativeAccessesTab.getAccessibleContext().setAccessibleDescription("");

        matrixPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Absolute Matrix Representation", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Arial", 1, 14))); // NOI18N
        matrixPanel.setMinimumSize(new java.awt.Dimension(450, 450));
        matrixPanel.setPreferredSize(new java.awt.Dimension(450, 450));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 6.0;
        getContentPane().add(matrixPanel, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        getContentPane().add(filler1, gridBagConstraints);

        statisticsPanel.setMinimumSize(new java.awt.Dimension(450, 100));
        statisticsPanel.setName("");
        statisticsPanel.setPreferredSize(new java.awt.Dimension(450, 100));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        getContentPane().add(statisticsPanel, gridBagConstraints);

        fileMenu.setText("File");

        openFileMenuItem.setIcon(new javax.swing.ImageIcon(getClass().getResource("/view/16px-Document-open.png"))); // NOI18N
        openFileMenuItem.setText("Open File...");
        openFileMenuItem.setPreferredSize(new java.awt.Dimension(150, 22));
        fileMenu.add(openFileMenuItem);
        fileMenu.add(seperator);

        exitMenuItem.setIcon(new javax.swing.ImageIcon(getClass().getResource("/view/16px-System-log-out.png"))); // NOI18N
        exitMenuItem.setText("Exit");
        exitMenuItem.setToolTipText("");
        fileMenu.add(exitMenuItem);

        menuBar.add(fileMenu);

        helpMenu.setText("Help");

        docMenuItem.setIcon(new javax.swing.ImageIcon(getClass().getResource("/view/16px-Help-browser.png"))); // NOI18N
        docMenuItem.setText("Help Contents");
        docMenuItem.setPreferredSize(new java.awt.Dimension(150, 22));
        helpMenu.add(docMenuItem);

        aboutMenuItem.setIcon(new javax.swing.ImageIcon(getClass().getResource("/view/16px-About.png"))); // NOI18N
        aboutMenuItem.setText("About");
        helpMenu.add(aboutMenuItem);

        menuBar.add(helpMenu);

        setJMenuBar(menuBar);

        pack();
    }// </editor-fold>//GEN-END:initComponents

    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        /*
         * Set the Nimbus look and feel
         */
        //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
        /*
         * If Nimbus (introduced in Java SE 6) is not available, stay with the
         * default look and feel. For details see
         * http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html
         */
        try {
            for (javax.swing.UIManager.LookAndFeelInfo info : javax.swing.UIManager.getInstalledLookAndFeels()) {
                if ("Nimbus".equals(info.getName())) {
                    javax.swing.UIManager.setLookAndFeel(info.getClassName());
                    break;
                }
            }
        } catch (ClassNotFoundException ex) {
            java.util.logging.Logger.getLogger(MainFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(MainFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(MainFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(MainFrame.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        /*
         * Create and display the form
         */
        java.awt.EventQueue.invokeLater(new Runnable() {

            @Override
            public void run() {
                new MainFrame().setVisible(true);
            }
        });
    }
    // Variables declaration - do not modify//GEN-BEGIN:variables
    public javax.swing.JDialog aboutDialog;
    private javax.swing.JMenuItem aboutMenuItem;
    private javax.swing.JPanel aboutPanel;
    private javax.swing.JScrollPane aboutScrollPane;
    private javax.swing.JTextArea aboutTextArea;
    private javax.swing.ButtonGroup dataStructuresButtonGroup;
    private javax.swing.JMenuItem docMenuItem;
    private javax.swing.JComboBox dropdownMenu;
    private javax.swing.JMenuItem exitMenuItem;
    public javax.swing.JFileChooser fileChooser;
    private javax.swing.JMenu fileMenu;
    private javax.swing.Box.Filler filler1;
    private javax.swing.Box.Filler filler2;
    private javax.swing.Box.Filler filler3;
    private javax.swing.JDialog helpDialog;
    private javax.swing.JMenu helpMenu;
    private javax.swing.JEditorPane helpPane;
    private javax.swing.JPanel helpPanel;
    private javax.swing.JScrollPane helpScrollPane;
    private javax.swing.JLabel labelToolBar;
    private javax.swing.JToggleButton loadButton;
    private javax.swing.ButtonGroup loadStoreButtonGroup;
    private javax.swing.JPanel matrixPanel;
    private javax.swing.JMenuBar menuBar;
    private javax.swing.JMenuItem openFileMenuItem;
    private javax.swing.JPanel patternDetailsPanel;
    private view.PatternOverviewPanel patternsOverviewPanel;
    private javax.swing.JPanel patternsTab;
    private javax.swing.JPanel piePanel;
    private view.RelativeAccessPanel relativeAccessPanel;
    private javax.swing.JTabbedPane relativeAccessesTab;
    private javax.swing.JPanel relativeJumpsTab;
    private javax.swing.JPopupMenu.Separator seperator;
    private view.StatisticsPanel statisticsPanel;
    private javax.swing.JToggleButton storeButton;
    private javax.swing.JScrollPane teamScrollPane;
    private javax.swing.JTextArea teamTextArea;
    private javax.swing.JToolBar toolbar;
    // End of variables declaration//GEN-END:variables

    public void updateDropdownMenu(List<String> names) {
        dropdownMenu.removeAllItems();
        for (String name : names) {
            dropdownMenu.addItem(name);
        }
        dropdownMenu.getItemAt(0);
        dropdownMenu.setSelectedIndex(0);
        loadButton.setSelected(true);
    }

    /**
     *
     * @param overallHits
     * @param overallMisses
     */
    public void updateOverallStatistics(long overallHits, long overallMisses) {
        statisticsPanel.setOverallStatistics(overallHits, overallMisses);
    }

    public void updateMatrixStatistics(int[] load, int[] store, String name) {
        statisticsPanel.setMatrixStatistics(load, store, name);
    }

    /**
     *
     * @param matrix implementation
     */
    public void addAbsoluteRepresentation(DataInput matrix) {
        MatrixPanel m = new MatrixPanel(matrix);
        JScrollPane scrollPane = new JScrollPane(m,
                ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS, ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        scrollPane.setPreferredSize(new java.awt.Dimension(450, 450));
        matrixPanel.add(scrollPane);
    }

    /**
     *
     */
    public void removeAbsoluteRepresentation() {
        matrixPanel.removeAll();
    }

    /**
     *
     * Adds the relative statistics + arrow for the specified access of the
     * matrix.
     *
     * @param jump
     */
    public void addArrowStatistics(RelativeJump jump) {
        ArrowStatisticsPanel arrow = new ArrowStatisticsPanel();
        // initialize Data
        arrow.setJumpDirections(jump.getX_move(), jump.getY_move(),
                (double) jump.getHits() / (double) (jump.getHits() + jump.getMisses()) * 100);
        arrow.setNumber(jump.getHits() + jump.getMisses());
        arrow.setHits(jump.getHits());
        arrow.setMisses(jump.getMisses());
        arrow.setSuccessrate((double) jump.getHits()
                / (double) (jump.getHits() + jump.getMisses()) * 100);
        relativeAccessPanel.addArrowStatisticsPanel(arrow);
    }

    /**
     *
     * Removes the all the rrelativeAccessPanells from the GUI.
     */
    public void removeArrowStatistics() {
        relativeAccessPanel.removeArrowStatistics();
        relativeAccessPanel.updateUI();
    }

    /**
     *
     * Adds a pie chart for the specified matrix to the GUI.
     *
     * @param matrix
     */
    public void addPieChart(DataInput matrix) {
        PieContainer pieContainer = new PieContainer(matrix);
        piePanel.add(pieContainer);
        piePanel.updateUI();
    }

    /**
     *
     * Removes the pie chart from the GUI.
     */
    public void removePieChart() {
        piePanel.removeAll();
    }

    /**
     *
     * @param s
     */
    public void addPattern(Pattern p) {
        patternsOverviewPanel.addPattern(p);
    }

    /**
     *
     */
    public void removePatterns() {
        patternsOverviewPanel.removePatterns();
    }

    public void updateDetailedPatternStatistics(Pattern p) {
        GridBagConstraints constraints = new GridBagConstraints();
        constraints.gridx = 1;
        constraints.gridy = 1;
        constraints.fill = GridBagConstraints.BOTH;
        constraints.weightx = 1.0;
        constraints.weighty = 1.0;

        patternDetailsPanel.removeAll();
        detailedPanel = new PatternDetailsPanel();
        detailedPanel.setPattern(p);

        patternDetailsPanel.add(detailedPanel, constraints);
        patternDetailsPanel.updateUI();
    }

    public void addSequencesToPattern(List<Sequence> seq) {
        if (detailedPanel != null) {
            detailedPanel.setSequences(seq);
        }
    }

    /**
     *
     * Adds a MatrixListener to all the combobox. ActionCommand equals the
     * number in the matrices array.
     *
     * @param al
     */
    public void addMatrixListener(ActionListener al) {
        dropdownMenu.addActionListener(al);
    }

    /**
     *
     * Adds a FileMenuListener to all the buttons.
     *
     * @param al
     */
    public void addFileMenuListener(ActionListener al) {
        openFileMenuItem.addActionListener(al);
        openFileMenuItem.setActionCommand("open-file");
        exitMenuItem.addActionListener(al);
        exitMenuItem.setActionCommand("exit");
    }

    /**
     *
     * @param al
     */
    public void addHelpMenuListener(ActionListener al) {
        docMenuItem.addActionListener(al);
        docMenuItem.setActionCommand("doc");
        aboutMenuItem.addActionListener(al);
        aboutMenuItem.setActionCommand("about");
    }

    /**
     *
     * @param al
     */
    public void addAccessModeListener(ActionListener al) {
        loadButton.addActionListener(al);
        loadButton.setActionCommand("load");
        storeButton.addActionListener(al);
        storeButton.setActionCommand("store");
    }

    public void showAboutDialog() {
        aboutDialog.setSize(465, 300);
        aboutDialog.setDefaultCloseOperation(javax.swing.JFrame.HIDE_ON_CLOSE);
        aboutDialog.setVisible(true);
    }
    
    public void showHelpDialog() {
        helpDialog.setSize(700, 600);
        helpDialog.setDefaultCloseOperation(javax.swing.JFrame.HIDE_ON_CLOSE);
        helpDialog.setVisible(true);
    }

    public JComboBox getDropDownMenu() {
        return dropdownMenu;
    }

    public JToggleButton getLoadButton() {
        return loadButton;
    }

    public RelativeAccessPanel getRelativeJumpPanel() {
        return relativeAccessPanel;
    }

    public PatternOverviewPanel getPatternsOverviewPanel() {
        return patternsOverviewPanel;
    }
}
