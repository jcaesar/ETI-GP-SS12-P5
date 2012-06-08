/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package data;

/**
 *
 * @author simon
 */
public class RelativeJump {

    private int x_move;
    private int y_move;
    private int hits;
    private int misses;

    public RelativeJump(int x_move, int y_move, int hits, int misses) {
        this.x_move = x_move;
        this.y_move = y_move;
        this.hits = hits;
        this.misses = misses;
    }

    /**
     * @return the x_move
     */
    public int getX_move() {
        return x_move;
    }

    /**
     * @param x_move the x_move to set
     */
    public void setX_move(int x_move) {
        this.x_move = x_move;
    }

    /**
     * @return the y_move
     */
    public int getY_move() {
        return y_move;
    }

    /**
     * @param y_move the y_move to set
     */
    public void setY_move(int y_move) {
        this.y_move = y_move;
    }

    /**
     * @return the hits
     */
    public int getHits() {
        return hits;
    }

    /**
     * @param hits the hits to set
     */
    public void setHits(int hits) {
        this.hits = hits;
    }

    /**
     * @return the misses
     */
    public int getMisses() {
        return misses;
    }

    /**
     * @param misses the misses to set
     */
    public void setMisses(int misses) {
        this.misses = misses;
    }
}
