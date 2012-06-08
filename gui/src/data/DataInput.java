package data;
import java.util.List;

/**
 *
 * @author Simon Wimmer
 */
public interface DataInput {
    byte[][] getAbsoluteMatrix(); // 0<->100% Misses, 1<->100% Hits
    List<RelativeJump> getRelativeJumps();
    int getHits(); // overall number of hits
    int getMisses(); // overall number of misses
}
