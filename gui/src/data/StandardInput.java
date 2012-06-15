package data;

import java.util.Collections;
import java.util.List;

/**
 *
 * @author Simon Wimmer
 */
public class StandardInput implements DataInput {

    private byte[][] absoluteStoreMatrix, absoluteLoadMatrix;
    private List<RelativeJump> relativeJumps;

    @Override
    public byte[][] getAbsoluteStoreMatrix() {
        return absoluteStoreMatrix;
    }

    @Override
    public byte[][] getAbsoluteLoadMatrix() {
        return absoluteLoadMatrix;
    }

    @Override
    public List<RelativeJump> getRelativeJumps() {
        return Collections.unmodifiableList(relativeJumps);
    }

    /**
     * @param absoluteMatrix the absoluteMatrix to set
     */
    public void setAbsoluteStoreMatrix(byte[][] absoluteMatrix) {
        this.absoluteStoreMatrix = absoluteMatrix;
    }

    /**
     * @param absoluteMatrix the absoluteMatrix to set
     */
    public void setAbsoluteLoadMatrix(byte[][] absoluteMatrix) {
        this.absoluteLoadMatrix = absoluteMatrix;
    }

    /**
     * @param relativeJumps the relativeJumps to set
     */
    public void setRelativeJumps(List<RelativeJump> relativeJumps) {
        this.relativeJumps = relativeJumps;
    }

    @Override
    public long[] getAbsoluteNumAccesses() {
        if (relativeJumps == null) {
            return new long[]{0, 0};
        }
        long hits = 0;
        long misses = 0;
        for (RelativeJump jump : relativeJumps) {
            hits += jump.getHits();
            misses += jump.getMisses();
        }
        return new long[]{hits, misses};
    }
}
