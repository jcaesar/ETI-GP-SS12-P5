package data;

import java.util.List;

/**
 *
 * @author Simon Wimmer Did get sort of obosolete by the time, but is still
 * there to keep the amount of code changes down. Now there ist only one
 * implementing class StandardInput, which provides some extra functionality.
 */
public interface DataInput {

    byte[][] getAbsoluteMatrix(); //0<->100% Misses, 1<->100% Hits

    int[] getAbsoluteNumAccesses(); //2-dim array, first hits, then misses

    String getName(); //name of matrix

    List<RelativeJump> getRelativeJumps();

    Pattern[] getPatterns();

    Sequence[] getSequences();
}
