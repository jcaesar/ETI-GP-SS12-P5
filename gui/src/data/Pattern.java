package data;

import java.util.List;

/**
 *
 * @author Simon Wimmer Represents a Pattern as specified in the
 * Dataformat-Documentation on
 * https://github.com/bucjac/ETI-GP-SS12-P5/wiki/Dateiformat
 */
public class Pattern {

    public int pid;
    private int numOccurrences;
    private int length;
    private List<RelativeJump> jumps;

    public Pattern(int pid, int numOccurrences, int length) {
        this.numOccurrences = numOccurrences;
        this.length = length;
        this.pid = pid;
    }

    @Override
    public String toString() {
        String s = "Pattern-ID: " + pid + " numOccurences: " + numOccurrences + " Length: " + length + "\nPattern-Jumps:";
        for (RelativeJump jump : jumps) {
            s += "\n" + jump;
        }
        return s;
    }

    /**
     * @return the numOccurrences
     */
    public int getNumOccurrences() {
        return numOccurrences;
    }

    /**
     * @param numOccurrences the numOccurrences to set
     */
    public void setNumOccurrences(int numOccurrences) {
        this.numOccurrences = numOccurrences;
    }

    /**
     * @return the length
     */
    public int getLength() {
        return length;
    }

    /**
     * @param length the length to set
     */
    public void setLength(int length) {
        this.length = length;
    }

    /**
     * @return the jumps
     */
    public List<RelativeJump> getJumps() {
        return jumps;
    }

    /**
     * @param jumps the jumps to set
     */
    public void setJumps(List<RelativeJump> jumps) {
        this.jumps = jumps;
    }

    public int getPID() {
        return pid;
    }
}
