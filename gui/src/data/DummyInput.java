package data;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 *
 * @author Simon Wimmer
 *
 * Jetzt mit variabler Anzahl von Zugriffsarten und variabler Zahl von
 * Zugriffen. Jetzt wird auch die Matrix entsprechend korrekt mit Werten von 0
 * bis 255 belegt.
 */
public class DummyInput implements DataInput {

    private byte matrix[][];
    private List<RelativeJump> jumps = new ArrayList<RelativeJump>();
    private int overallHits = 0;
    private int overallMisses = 0;

    public DummyInput() {
        final int MATRIX_X_SIZE = 150;
        final int MATRIX_Y_SIZE = 150;

        matrix = new byte[MATRIX_X_SIZE][MATRIX_Y_SIZE];
        Random rand = new Random();
        for (int i = 0; i < MATRIX_X_SIZE; i++) {
            for (int j = 0; j < MATRIX_Y_SIZE; j++) {
                matrix[i][j] = (byte) rand.nextInt(256);
            }
        }

        //Anzahl der Zugriffe pro Zugriffsart
        int[] sizes = new int[8];
        for (int i = 0; i < 8; i++) {
            sizes[i] = rand.nextInt(2000) + 100;
        }

        //Größte relative Bewegung
        final int MAX_MOVE = 100;

        int hits;
        for (int i = 0; i < sizes.length; i++) {
            hits = (int) (rand.nextDouble() * sizes[i]);
            jumps.add(new RelativeJump((int) (rand.nextDouble() * MAX_MOVE) - 10,
                    (int) (rand.nextDouble() * MAX_MOVE) - 50, hits, sizes[i] - hits));
            overallHits += hits;
            overallMisses += sizes[i] - hits;
        }
    }

    @Override
    public List<RelativeJump> getRelativeJumps() {
        return jumps;
    }

    @Override
    public byte[][] getAbsoluteStoreMatrix() {
        return matrix;
    }

    @Override
    public byte[][] getAbsoluteLoadMatrix() {
        return matrix;
    }

    @Override
    public int[] getAbsoluteNumAccesses() {
        throw new UnsupportedOperationException("Not supported yet.");
    }
}
