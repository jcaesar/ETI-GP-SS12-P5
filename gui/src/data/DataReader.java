package data;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Simon Wimmer
 */
public class DataReader {

    public static DataInput[][] readData(File file) throws FileNotFoundException, IOException {
        FileInputStream fis = new FileInputStream(file);
        int num_matrix = readHeader(fis);
        MatrixHeader[] headers = readMatrixHeaders(num_matrix, fis);
        StandardInput[][] inputs = readMatrices(fis, headers);
        return inputs;
    }

    private static int readHeader(FileInputStream fis) throws IOException {
        int id1 = fis.read();
        int id2 = fis.read();
        if (id1 != 175 && id2 != 254) {
            throw new IOException("Wrong file format!");
        }
        fis.read(); //File Version ignoriert
        return fis.read();
    }

    private static int readTwoBytesSigned(FileInputStream fis) throws IOException {
        int n = readNBytes(fis, 2);
        if (n >= 0x8000) {
            n = (n & 0x7fff) - 0x8000;
        }
        return n;
    }

    private static int readTwoBytes(FileInputStream fis) throws IOException {
        return readNBytes(fis, 2);
    }

    private static int readFourBytes(FileInputStream fis) throws IOException {
        long r = readNBytesLong(fis, 4);
        return new Long(r).intValue();
    }

    private static int readNBytes(FileInputStream fis, int num_bytes) throws IOException {
        //Mehrere Bytes im Little Endian lesen
        int r = 0;
        int mult = 1;
        for (int i = 0; i < num_bytes; i++) {
            r = r + fis.read() * mult;
            mult *= 256;
        }
        return r;
    }

    private static long readNBytesLong(FileInputStream fis, int num_bytes) throws IOException {
        //Mehrere Bytes im Little Endian lesen
        long r = 0;
        long mult = 1;
        for (int i = 0; i < num_bytes; i++) {
            r = r + fis.read() * mult;
            mult *= 256;
        }
        return r;
    }

    private static long readEightBytes(FileInputStream fis) throws IOException {
        long r = 0;
        long mult = 1;
        for (int i = 0; i < 8; i++) {
            r = r + fis.read() * mult;
            mult *= 256;
        }
        return r;
    }

    private static MatrixHeader[] readMatrixHeaders(int num_matrix, FileInputStream fis) throws IOException {
        MatrixHeader[] headers = new MatrixHeader[num_matrix];
        for (int i = 0; i < num_matrix; i++) {
            headers[i] = new MatrixHeader();
            headers[i].setySize(readTwoBytesSigned(fis)); //m <> Anzahl der Zeilen <> Größe in y-Richtung
            headers[i].setxSize(readTwoBytesSigned(fis)); //n <> Anzahl der Spalten <> Größe in x-Richtung
            if (headers[i].getxSize() <= 0 || headers[i].getySize() <= 0) {
                throw new IOException("Illegal matrix size in matrix header!");
            }
            headers[i].setNumLoadJumps(fis.read());
            headers[i].setNumStoreJumps(fis.read());
            headers[i].setNumPatterns(fis.read());
            headers[i].setNumSequences(fis.read());
            //headers[i].setDataType(fis.read()); Datentyp rausgenommen
            headers[i].setAddr(readEightBytes(fis));
            headers[i].setMibAddr(readFourBytes(fis));
            headers[i].setLoadHits(readFourBytes(fis));
            headers[i].setLoadMisses(readFourBytes(fis));
            headers[i].setStoreHits(readFourBytes(fis));
            headers[i].setStoreMisses(readFourBytes(fis));
            //headers[i].printInfo();
        }
        return headers;
    }

    private static StandardInput[][] readMatrices(FileInputStream fis, MatrixHeader[] headers) throws IOException {
        StandardInput[][] input = new StandardInput[headers.length][2];
        for (int i = 0; i < headers.length; i++) {
            input[i] = new StandardInput[2];

            input[i][0] = new StandardInput();
            input[i][0].setHits(headers[i].getLoadHits());
            input[i][0].setMisses(headers[i].getLoadMisses());

            input[i][1] = new StandardInput();
            input[i][1].setHits(headers[i].getStoreHits());
            input[i][1].setMisses(headers[i].getStoreMisses());



            input[i][0].setAbsoluteMatrix(readByteMatrix(fis, headers[i]));

            //System.out.println(Long.toHexString(fis.getChannel().position()));

            input[i][1].setAbsoluteMatrix(readByteMatrix(fis, headers[i]));

            //System.out.println(Long.toHexString(fis.getChannel().position()));

            input[i][0].setRelativeJumps(readRelativeJumps(fis, headers[i].getNumLoadJumps()));

            //System.out.println(Long.toHexString(fis.getChannel().position()));

            input[i][1].setRelativeJumps(readRelativeJumps(fis, headers[i].getNumStoreJumps()));

            //System.out.println(Long.toHexString(fis.getChannel().position()));

            Pattern[] patterns = readPatterns(fis, headers[i].getNumPatterns());
            input[i][0].setPatterns(patterns);

            //System.out.println(Long.toHexString(fis.getChannel().position()));

            input[i][0].setSequences(readSequences(fis, headers[i].getNumSequences(), patterns));

            //System.out.println(Long.toHexString(fis.getChannel().position()));

            String name = readName(fis);
            input[i][0].setName(name);
            input[i][1].setName(name);
        }
        return input;
    }

    private static byte[][] readByteMatrix(FileInputStream fis, MatrixHeader matrixHeader) throws IOException {
        byte[] b = new byte[matrixHeader.getxSize() * matrixHeader.getySize()];
        fis.read(b, 0, b.length);
        byte[][] r = new byte[matrixHeader.getxSize()][matrixHeader.getySize()];
        for (int x = 0; x < matrixHeader.getxSize(); x++) {
            for (int y = 0; y < matrixHeader.getySize(); y++) {
                r[x][y] = b[x + y * matrixHeader.getxSize()];
            }
        }
        return r;
    }

    private static List<RelativeJump> readRelativeJumps(FileInputStream fis, int numJumps) throws IOException {
        List<RelativeJump> jumps = new ArrayList<RelativeJump>();
        int x_move, y_move, hits, misses;
        for (int i = 0; i < numJumps; i++) {
            y_move = readTwoBytesSigned(fis); //M-Offset <> Zeilen-Sprung <> y_move
            x_move = readTwoBytesSigned(fis); //N-Offset <> Spalten-Sprung <> x_move
            hits = readFourBytes(fis);
            misses = readFourBytes(fis);
            jumps.add(new RelativeJump(x_move, y_move, hits, misses));
        }
        return jumps;
    }

    private static Pattern[] readPatterns(FileInputStream fis, int numPatterns) throws IOException {
        Pattern[] patterns = new Pattern[numPatterns];
        int pid, numOcc, length;
        for (int i = 0; i < numPatterns; i++) {
            pid = fis.read();
            numOcc = readFourBytes(fis);
            length = readTwoBytes(fis);
            patterns[i] = new Pattern(pid, numOcc, length);
            patterns[i].setJumps(readRelativeJumps(fis, length));
            //System.out.println(patterns[i]);
        }
        return patterns;
    }

    private static Sequence[] readSequences(FileInputStream fis, int numSequences, Pattern[] patterns) throws IOException {
        Sequence[] sequences = new Sequence[numSequences];
        int pid, numOcc, repetitions, nextAccessMOffset, nextAccessNOffset, nextPID;
        for (int i = 0; i < numSequences; i++) {
            pid = fis.read();
            numOcc = readFourBytes(fis);
            repetitions = readTwoBytes(fis);
            nextAccessMOffset = readTwoBytes(fis);
            nextAccessNOffset = readTwoBytes(fis);
            sequences[i] = new Sequence(find(pid, patterns), numOcc, repetitions, nextAccessMOffset, nextAccessNOffset);
            nextPID = fis.read();
            sequences[i].setNextPattern(nextPID == 0xff ? null : find(nextPID, patterns));
        }
        return sequences;
    }

    private static Pattern find(int pid, Pattern[] patterns) {
        for (int i = 0; i < patterns.length; i++) {
            if (patterns[i].pid == pid) {
                return patterns[i];
            }
        }
        return null;
    }

    private static String readName(FileInputStream fis) throws IOException {
        String s = "";
        int b = fis.read();
        int i = 0;
        while (b != 0 && b != -1) {
            s += (char) b;
            b = fis.read();
            i++;
        }
        if (b == -1) {
            throw new IOException("Error while reading Matrix Name!" + i);
        }
        return s;
    }
}
