package data;

import java.util.List;

/**
 *
 * @author Simon Wimmer
 */
public interface DataInput {

    byte[][] getAbsoluteStoreMatrix(); //0<->100% Misses, 1<->100% Hits

    byte[][] getAbsoluteLoadMatrix(); //0<->100% Misses, 1<->100% Hits

    long[] getAbsoluteNumAccesses(); //2-elementiges Array, zuerst Hits, dann Misses

    List<RelativeJump> getRelativeJumps();
}
