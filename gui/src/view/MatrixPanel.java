package view;

import data.DataInput;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.event.AdjustmentEvent;
import java.awt.event.AdjustmentListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.awt.geom.AffineTransform;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

/**
 *
 * @author Thomas
 */
public class MatrixPanel extends JPanel {

    private Color[][] colors;
    private double dx, dy;
    private double scale;
    private double iScale;
    private boolean init, changeFromScroll;

    public MatrixPanel(DataInput matrix) {
        super();
        init = true;

        //Calculate colors
        byte[][] pixel = matrix.getAbsoluteMatrix();
        colors = new Color[pixel.length][pixel[0].length];
        for (int i = 0; i < pixel.length; i++) {
            for (int j = 0; j < pixel[0].length; j++) {
                byte color = pixel[i][j];


                if (color == 127) {
                    colors[i][j] = new Color(255, 255, 0);
                } else if (color == -1) {
                    colors[i][j] = new Color(255, 255, 255);
                } else if (color >= 0) {
                    colors[i][j] = new Color(255, color * 2, 0);
                } else {
                    colors[i][j] = new Color(254 - (128 + color) * 2, 255, 0);
                }
            }
        }

        this.addMouseWheelListener(new MouseWheelListener() {

            @Override
            public void mouseWheelMoved(MouseWheelEvent e) {
                zoom(e);
            }
        });

        scale = 1;
        dx = 0;
        dy = 0;
        changeFromScroll = true;
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        JScrollPane scroll = (JScrollPane) getParent().getParent();

        //First paint
        if (init) {
            init = false;
            scale = Math.min((double) this.getWidth() / colors.length, (double) this.getHeight() / colors[0].length);
            iScale = scale;


            scroll.getVerticalScrollBar().addAdjustmentListener(new AdjustmentListener() {

                @Override
                public void adjustmentValueChanged(AdjustmentEvent ae) {
                    MatrixPanel.this.repaint();
                }
            });
            scroll.getHorizontalScrollBar().addAdjustmentListener(new AdjustmentListener() {

                @Override
                public void adjustmentValueChanged(AdjustmentEvent ae) {
                    MatrixPanel.this.repaint();
                }
            });


            repaint();
        }

        //Repaint for changes from Scrollbar
        if (changeFromScroll && !(scroll.getHorizontalScrollBar().getMaximum() - scroll.getHorizontalScrollBar().getVisibleAmount() == 0) && !(scroll.getVerticalScrollBar().getMaximum() - scroll.getVerticalScrollBar().getVisibleAmount() == 0)) {
            dx = ((double) colors.length * ((iScale / scale) - 1)) * ((double) scroll.getHorizontalScrollBar().getValue() / (scroll.getHorizontalScrollBar().getMaximum() - scroll.getHorizontalScrollBar().getVisibleAmount()));
            dy = ((double) colors[0].length * ((iScale / scale) - 1)) * ((double) scroll.getVerticalScrollBar().getValue() / (scroll.getVerticalScrollBar().getMaximum() - scroll.getVerticalScrollBar().getVisibleAmount()));
        }

        changeFromScroll = true;

        Graphics2D g2 = (Graphics2D) g;
        g2.clearRect(0, 0, this.getWidth(), this.getHeight());

        AffineTransform tx = new AffineTransform();
        tx.translate(dx * scale, dy * scale);
        tx.scale(scale, scale);
        g2.setTransform(tx);


        for (int i = (int) -dx; i < Math.min(-dx + (iScale / scale) * colors.length, colors.length); i++) {
            for (int j = (int) -dy; j < Math.min(-dy + (iScale / scale) * colors[0].length, colors[0].length); j++) {

                g2.setColor(colors[i][j]);
                g2.fillRect(i, j, 1, 1);
            }
        }
    }

    private void zoom(MouseWheelEvent e) {
        if (e.getScrollType() == MouseWheelEvent.WHEEL_UNIT_SCROLL) {
            JScrollPane scroll = (JScrollPane) getParent().getParent();

            //Calculate Scale
            this.scale = scale * Math.exp(.1 * -e.getWheelRotation());
            this.scale = Math.max(scale, iScale);

            //Check borders
            dx = Math.max(dx, (double) colors.length * ((iScale / scale) - 1));
            dy = Math.max(dy, (double) colors[0].length * ((iScale / scale) - 1));

            //Adjust Scrollbar
            scroll.getHorizontalScrollBar().setValue((int) (dx
                    * (scroll.getHorizontalScrollBar().getMaximum() - scroll.getHorizontalScrollBar().getVisibleAmount())
                    / ((double) colors.length * ((iScale / scale) - 1))));
            scroll.getVerticalScrollBar().setValue((int) (dy
                    * (scroll.getVerticalScrollBar().getMaximum() - scroll.getVerticalScrollBar().getVisibleAmount())
                    / ((double) colors[0].length * ((iScale / scale) - 1))));


            this.setPreferredSize(new java.awt.Dimension((int) (colors.length * scale), (int) (colors[0].length * scale)));
            this.revalidate();
            changeFromScroll = false;
            this.repaint();
        }
    }
}
