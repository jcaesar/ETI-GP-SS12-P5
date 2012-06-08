/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package view;

import java.awt.Color;
import java.awt.GridBagConstraints;
import java.text.DecimalFormat;
import java.util.List;

/**
 *
 * @author philip
 */
public class ArrowStatisticsPanel extends javax.swing.JPanel {

    /**
     * Creates new form ArrowStatisticsPanel
     */
    public ArrowStatisticsPanel() {
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

        jumpDirections = new javax.swing.JLabel();
        successrate = new javax.swing.JLabel();
        numberLabel = new javax.swing.JLabel();
        number = new javax.swing.JLabel();
        hitsLabel = new javax.swing.JLabel();
        hits = new javax.swing.JLabel();
        missesLabel = new javax.swing.JLabel();
        misses = new javax.swing.JLabel();
        filler1 = new javax.swing.Box.Filler(new java.awt.Dimension(0, 5), new java.awt.Dimension(0, 5), new java.awt.Dimension(32767, 5));

        setBorder(new javax.swing.border.SoftBevelBorder(javax.swing.border.BevelBorder.RAISED));
        setMaximumSize(new java.awt.Dimension(300, 300));
        setMinimumSize(new java.awt.Dimension(80, 120));
        setPreferredSize(new java.awt.Dimension(80, 120));
        setLayout(new java.awt.GridBagLayout());

        jumpDirections.setFont(new java.awt.Font("Tahoma", 1, 12)); // NOI18N
        jumpDirections.setText(" ( | )");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.ipadx = 10;
        gridBagConstraints.ipady = 10;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        add(jumpDirections, gridBagConstraints);

        successrate.setText("%");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.ipadx = 5;
        gridBagConstraints.ipady = 10;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHEAST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        add(successrate, gridBagConstraints);

        numberLabel.setText("   #: ");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 2.0;
        gridBagConstraints.weighty = 1.0;
        add(numberLabel, gridBagConstraints);

        number.setText("0");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        add(number, gridBagConstraints);

        hitsLabel.setText("   Hits: ");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 2.0;
        gridBagConstraints.weighty = 1.0;
        add(hitsLabel, gridBagConstraints);

        hits.setText("0");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        add(hits, gridBagConstraints);

        missesLabel.setText("   Misses: ");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 6;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.weightx = 2.0;
        gridBagConstraints.weighty = 1.0;
        add(missesLabel, gridBagConstraints);

        misses.setText("0");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 6;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        add(misses, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 7;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        add(filler1, gridBagConstraints);
    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.Box.Filler filler1;
    private javax.swing.JLabel hits;
    private javax.swing.JLabel hitsLabel;
    private javax.swing.JLabel jumpDirections;
    private javax.swing.JLabel misses;
    private javax.swing.JLabel missesLabel;
    private javax.swing.JLabel number;
    private javax.swing.JLabel numberLabel;
    private javax.swing.JLabel successrate;
    // End of variables declaration//GEN-END:variables
    private ArrowPanel arrow;
    
    private void createArrowPanel(int x, int y, double rate) {
        arrow = new ArrowPanel(x,y,rate);
        java.awt.GridBagConstraints gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.weightx = 1;
        gridBagConstraints.weighty = 1;
        // gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.anchor = GridBagConstraints.CENTER;
        gridBagConstraints.fill = GridBagConstraints.VERTICAL;
        add(arrow, gridBagConstraints);
    }
    
    public void setJumpDirections(int x, int y, double rate) {
        jumpDirections.setText(" (" + x + "|" + y + ")");
        createArrowPanel(x,y,rate);
    }
    
    public void setNumber(int n) {
        number.setText(String.valueOf(n));
    }
    
    public void setHits(int hits) {
        this.hits.setText(String.valueOf(hits));
    }
    
    public void setMisses(int misses) {
        this.misses.setText(String.valueOf(misses));
    }

    public void setSuccessrate(double rate) {
        DecimalFormat f = new DecimalFormat("#0.00");
        // Color c = new Color((int) (255*(100-rate))/100, (int) (255*rate)/100,0);
        // successrate.setForeground(c);
        successrate.setText(String.valueOf(f.format(rate)) + "%");
    } 
}
