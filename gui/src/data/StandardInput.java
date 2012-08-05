package data;

import java.util.Collections;
import java.util.List;

/**
 *
 * @author Simon Wimmer
 */
public class StandardInput implements DataInput {

    private byte[][] absoluteMatrix;
    private List<RelativeJump> relativeJumps;
    private int Hits, Misses;
    private String name;
    private Sequence[] sequences;
    private Pattern[] patterns;

    /**
     * @param name the name to set
     */
    public void setName(String name) {
        this.name = name;
    }

    public void printInfo() {
        System.out.println("Name " + getName());
        System.out.println("Hits " + Hits);
        System.out.println("Misses " + Misses);
        System.out.println("Num Jumps " + relativeJumps.size());
        System.out.println("Jumps:");
        for (RelativeJump jump : relativeJumps) {
            System.out.println(jump);
        }
        System.out.println("Sequences:");
        if (sequences == null) {
            System.out.println("No Sequences found.");
            return;
        }
        for (Sequence seq : sequences) {
            System.out.println(seq == null ? "No Sequence found" : seq);
        }
    }

    /**
     * @return the absoluteMatrix
     */
    @Override
    public byte[][] getAbsoluteMatrix() {
        return absoluteMatrix;
    }

    /**
     * @param absoluteMatrix the absoluteMatrix to set
     */
    public void setAbsoluteMatrix(byte[][] absoluteMatrix) {
        this.absoluteMatrix = absoluteMatrix;
    }

    /**
     * @return the relativeJumps
     */
    @Override
    public List<RelativeJump> getRelativeJumps() {
        return Collections.unmodifiableList(relativeJumps);
    }

    /**
     * @param relativeJumps the relativeJumps to set
     */
    public void setRelativeJumps(List<RelativeJump> relativeJumps) {
        this.relativeJumps = relativeJumps;
    }

    /**
     * @return the Hits
     */
    public int getHits() {
        return Hits;
    }

    /**
     * @param Hits the Hits to set
     */
    public void setHits(int Hits) {
        this.Hits = Hits;
    }

    /**
     * @return the Misses
     */
    public int getMisses() {
        return Misses;
    }

    /**
     * @param Misses the Misses to set
     */
    public void setMisses(int Misses) {
        this.Misses = Misses;
    }

    /**
     * @return the name
     */
    @Override
    public String getName() {
        return name;
    }

    @Override
    public int[] getAbsoluteNumAccesses() {
        return new int[]{Hits, Misses};
    }

    /**
     * @return the sequences
     */
    @Override
    public Sequence[] getSequences() {
        return sequences;
    }

    /**
     * @param sequences the sequences to set
     */
    public void setSequences(Sequence[] sequences) {
        this.sequences = sequences;
    }

    /**
     * @return the patterns
     */
    @Override
    public Pattern[] getPatterns() {
        return patterns;
    }

    /**
     * @param patterns the patterns to set
     */
    public void setPatterns(Pattern[] patterns) {
        this.patterns = patterns;
    }
}
