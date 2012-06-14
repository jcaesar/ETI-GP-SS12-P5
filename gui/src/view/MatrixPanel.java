package view;

import data.DataInput;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.awt.geom.AffineTransform;
import javax.swing.JPanel;

/**
 *
 * @author Thomas
 */
public class MatrixPanel extends JPanel {

    private Color[][] colors;
    private Point start, end;
    private double dx, dy;
    private double scale;
    private boolean init;
    private DataInput matrix;

    public MatrixPanel(DataInput matrix) {
        super();
        init = true;


        this.matrix = matrix;

        byte[][] pixel = matrix.getAbsoluteLoadMatrix();
        colors = new Color[pixel.length][pixel[0].length];
        for (int i = 0; i < pixel.length; i++) {
            for (int j = 0; j < pixel[0].length; j++) {
                byte color = pixel[i][j];
                System.out.println(i + "," + j + ":" + (128 + color) * 2);

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



        start = new Point();
        this.addMouseMotionListener(new MouseMotionListener() {

            @Override
            public void mouseDragged(MouseEvent e) {
                moveCamera(e);
            }

            @Override
            public void mouseMoved(MouseEvent e) {
            }
        });
        this.addMouseListener(new MouseListener() {

            @Override
            public void mouseClicked(MouseEvent e) {
            }

            @Override
            public void mousePressed(MouseEvent e) {
                start = e.getPoint();
            }

            @Override
            public void mouseReleased(MouseEvent e) {
            }

            @Override
            public void mouseEntered(MouseEvent e) {
            }

            @Override
            public void mouseExited(MouseEvent e) {
            }
        });

        this.addMouseWheelListener(new MouseWheelListener() {

            @Override
            public void mouseWheelMoved(MouseWheelEvent e) {
                zoom(e);
            }
        });

        //System.out.println("width: " + this.getWidth());
        scale = 1;
        dx = 0;
        dy = 0;
    }

    @Override
    protected void paintComponent(Graphics g) {
        if (init) {
            init = false;
            scale = Math.min((double) this.getWidth() / colors.length, (double) this.getHeight() / colors[0].length);
            repaint();
        }
        Graphics2D g2 = (Graphics2D) g;
        g2.clearRect(0, 0, this.getWidth(), this.getHeight());

        AffineTransform tx = new AffineTransform();
        tx.translate(dx, dy);
        System.out.println("dx: " + dx + ", dy: " + dy + ", scale:" + scale);
        tx.scale(scale, scale);
        g2.setTransform(tx);

        System.out.println("x:" + start.getX() + "," + dx + "; y:" + start.getY() + "," + dy);
        for (int i = 0; i < colors.length; i++) {
            for (int j = 0; j < colors[0].length; j++) {
                g2.setColor(colors[i][j]);
                g2.fillRect(i, j, 1, 1);
            }
        }


    }

    private void moveCamera(MouseEvent e) {

        end = e.getPoint();
        System.out.println("end: " + end.toString());
        System.out.println("start: " + end.toString());
        dx = dx + end.getX() - start.getX();
        //dx = Math.max(0, dx + end.getX() - start.getX());
        //dx = Math.min(dx, this.getWidth() - colors.length*scale);
        dy = dy + end.getY() - start.getY();

        start = end;

        this.repaint();
    }

    private void zoom(MouseWheelEvent e) {
        if (e.getScrollType() == MouseWheelEvent.WHEEL_UNIT_SCROLL) {

            //this.scale += (.1 * e.getWheelRotation());
            this.scale = scale * Math.exp(.1 * -e.getWheelRotation());
            this.scale = Math.max(0.00001, this.scale);
            this.repaint();
        }
    }
}
