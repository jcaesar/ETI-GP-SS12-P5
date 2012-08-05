package data;

import java.util.List;

/**
 *
 * @author Simon Wimmer
 */
public interface DataInput {

    byte[][] getAbsoluteMatrix(); //0<->100% Misses, 1<->100% Hits

    int[] getAbsoluteNumAccesses(); //2-elementiges Array, zuerst Hits, dann Misses

    String getName(); //Name der Matrix

    List<RelativeJump> getRelativeJumps();

    Sequence[] getSequences();

    Pattern[] getPatterns();
}
