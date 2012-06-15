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

    public static DataInput[] readData(File file) throws FileNotFoundException, IOException {
        FileInputStream fis = new FileInputStream(file);
        int num_matrix = readHeader(fis);
        MatrixHeader[] headers = readMatrixHeaders(num_matrix, fis);
        StandardInput[] inputs = readMatrices(fis, headers);
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
            headers[i].setxSize(readTwoBytesSigned(fis));
            headers[i].setySize(readTwoBytesSigned(fis));
            if (headers[i].getxSize() <= 0 || headers[i].getySize() <= 0) {
                throw new IOException("Illegal matrix size in matrix header!");
            }
            headers[i].setNumJumps(fis.read());
            //headers[i].setDataType(fis.read()); Datentyp rausgenommen
            headers[i].setAddr(readEightBytes(fis));
            headers[i].setMibAddr(readFourBytes(fis));
        }
        return headers;
    }

    private static StandardInput[] readMatrices(FileInputStream fis, MatrixHeader[] headers) throws IOException {
        StandardInput[] input = new StandardInput[headers.length];
        for (int i = 0; i < headers.length; i++) {
            input[i] = new StandardInput();

            fis.getChannel().position(headers[i].getMibAddr());

            input[i].setAbsoluteLoadMatrix(readByteMatrix(fis, headers[i]));

            input[i].setAbsoluteStoreMatrix(readByteMatrix(fis, headers[i]));

            input[i].setRelativeJumps(readRelativeJumps(fis, headers[i]));

            headers[i].setName(readName(fis));
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

    private static List<RelativeJump> readRelativeJumps(FileInputStream fis, MatrixHeader matrixHeader) throws IOException {
        List<RelativeJump> jumps = new ArrayList<RelativeJump>();
        int x_move, y_move, hits, misses;
        for (int i = 0; i < matrixHeader.getNumJumps(); i++) {
            x_move = readTwoBytesSigned(fis);
            y_move = readTwoBytesSigned(fis);
            hits = readFourBytes(fis);
            misses = readFourBytes(fis);
            jumps.add(new RelativeJump(x_move, y_move, hits, misses));
        }
        return jumps;
    }

    private static String readName(FileInputStream fis) throws IOException {
        String s = "";
        int b = fis.read();
        while (b != 0 && b != -1) {
            s += (char) b;
            b = fis.read();
        }
        if (b == -1) {
            throw new IOException("Error while reading Matrix Name!");
        }
        return s;
    }
}
