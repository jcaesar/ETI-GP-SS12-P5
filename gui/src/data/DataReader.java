package eti.data;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Simon Wimmer
 * Provides utility method readData(File file) for reading files that conform
 * to the Data-Format specified in 
 * https://github.com/bucjac/ETI-GP-SS12-P5/wiki/Dateiformat
 */
public class DataReader {
    /**
     * 
     * @param file A file conforming to the Data-Format specification
     * @return An two-dimensional array of StandardInput objects;
     * The array contains an array 
     * [StandardInput object for load, StandardInput object for store] for
     * every Matrix
     * @throws FileNotFoundException
     * @throws IOException 
     */
    public static StandardInput[][] readData(File file) throws FileNotFoundException, IOException{
        FileInputStream fis=new FileInputStream(file);
        
        //Check for correct file header and read number of matrices that are 
        //specified in the file
        int num_matrix=readHeader(fis);
        
        //First read all matrix headers
        MatrixHeader[] headers=readMatrixHeaders(num_matrix,fis);
        
        //Second read all corresponding information, i.e. absolute matrices,
        //relative jumps, patterns and sequences
        StandardInput[][] inputs=readMatrices(fis,headers);
        
        return inputs;
    }
    
    
    private static int readHeader(FileInputStream fis) throws IOException{
        //Check for correct File-ID
        int id1=fis.read();
        int id2=fis.read();
        if(id1!=175&&id2!=254) throw new IOException("Wrong file format!");
        
        fis.read(); //Ingore file version
        return fis.read(); //Return number of matrices
    }
    
    //A lot of utility methods for reading signed and unsigned Integers in
    //little endian format
    private static int readTwoBytesSigned(FileInputStream fis) throws IOException{
        int n=readNBytes(fis,2);
        if(n>=0x8000){
            n=(n&0x7fff)-0x8000;
        }
        return n;
    }
    private static int readTwoBytes(FileInputStream fis) throws IOException{
        return readNBytes(fis,2);
    }
    private static int readFourBytes(FileInputStream fis) throws IOException{
        long r= readNBytesLong(fis,4);
        return new Long(r).intValue();
    }
    
    private static int readNBytes(FileInputStream fis, int num_bytes) throws IOException{
        //Read multiple bytes in little endian
        int r=0;
        int mult=1;
        for(int i=0; i<num_bytes; i++){
            r=r+fis.read()*mult;
            mult*=256;
        }
        return r;
    }
    private static long readNBytesLong(FileInputStream fis, int num_bytes) throws IOException{
        //read multiple bytes in little endian
        long r=0;
        long mult=1;
        for(int i=0; i<num_bytes; i++){
            r=r+fis.read()*mult;
            mult*=256;
        }
        return r;
    }
    private static long readEightBytes(FileInputStream fis) throws IOException{
        long r=0;
        long mult=1;
        for(int i=0; i<8; i++){
            r=r+fis.read()*mult;
            mult*=256;
        }
        return r;
    }
    
    //End of utility methods

    
    private static MatrixHeader[] readMatrixHeaders(int num_matrix, FileInputStream fis) throws IOException {
        MatrixHeader[] headers = new MatrixHeader[num_matrix];
        for (int i=0; i<num_matrix; i++){
            headers[i]=new MatrixHeader();
            headers[i].setySize(readTwoBytesSigned(fis)); //m <> number of lines <> size in y-direction
            headers[i].setxSize(readTwoBytesSigned(fis)); //n <> number of colons <> size in x-direction
            
            //Check for legal size
            if (headers[i].getxSize()<=0||headers[i].getySize()<=0){
                throw new IOException("Illegal matrix size in matrix header!");
            }
            
            //Read Header information according to specification
            headers[i].setNumLoadJumps(fis.read());
            headers[i].setNumStoreJumps(fis.read());
            headers[i].setNumPatterns(fis.read());
            headers[i].setNumSequences(fis.read());
            headers[i].setAddr(readEightBytes(fis));
            headers[i].setMibAddr(readFourBytes(fis));
            headers[i].setLoadHits(readFourBytes(fis));
            headers[i].setLoadMisses(readFourBytes(fis));
            headers[i].setStoreHits(readFourBytes(fis));
            headers[i].setStoreMisses(readFourBytes(fis));
        }
        return headers;
    }

    
    private static StandardInput[][] readMatrices(FileInputStream fis, MatrixHeader[] headers) throws IOException {
        //For every matrix first element in array ist loads, second is stores
        StandardInput[][] input=new StandardInput[headers.length][2];
        
        for(int i=0; i<headers.length; i++){
            input[i]=new StandardInput[2];
            
            //Copy some general info from matrix headers
            input[i][0]=new StandardInput();
            input[i][0].setHits(headers[i].getLoadHits());
            input[i][0].setMisses(headers[i].getLoadMisses());
            
            input[i][1]=new StandardInput();
            input[i][1].setHits(headers[i].getStoreHits());
            input[i][1].setMisses(headers[i].getStoreMisses());
            
            
            //Read absolute matrix for load
            input[i][0].setAbsoluteMatrix(readByteMatrix(fis,headers[i]));
            
            
            //Read absolute matrix for store
            input[i][1].setAbsoluteMatrix(readByteMatrix(fis,headers[i]));
            
            
            //Read relative jumps for loads
            input[i][0].setRelativeJumps(readRelativeJumps(fis,headers[i].getNumLoadJumps()));
            
            
            //Read relative jumps for loads
            input[i][1].setRelativeJumps(readRelativeJumps(fis,headers[i].getNumStoreJumps()));
            
            
            //Patterns and Sequences do not distinguish between Loads/Stores,
            //so only save them for Load
            Pattern[] patterns=readPatterns(fis, headers[i].getNumPatterns());
            input[i][0].setPatterns(patterns);
            
            input[i][0].setSequences(readSequences(fis, headers[i].getNumSequences(), patterns));
            
            
            //Read the matrix's name represented as C-style string
            String name=readName(fis);
            input[i][0].setName(name);
            input[i][1].setName(name);
        }
        return input;
    }

    private static byte[][] readByteMatrix(FileInputStream fis, MatrixHeader matrixHeader) throws IOException {
       byte[] b=new byte[matrixHeader.getxSize()*matrixHeader.getySize()];
       
       fis.read(b, 0, b.length);
       
       byte[][] r=new byte[matrixHeader.getxSize()][matrixHeader.getySize()];
       
       for(int x=0; x<matrixHeader.getxSize(); x++){
           for(int y=0; y<matrixHeader.getySize(); y++){
               r[x][y]=b[x+y*matrixHeader.getxSize()];
           }
       }
       return r;
    }
    

    private static List<RelativeJump> readRelativeJumps(FileInputStream fis, int numJumps) throws IOException {
        List<RelativeJump> jumps=new ArrayList<RelativeJump>();
        
        int x_move, y_move, hits, misses;
        
        //Read specified number of jumps according to specification
        for(int i=0; i<numJumps; i++){
            y_move=readTwoBytesSigned(fis); //M-Offset <> line offset <> y_move
            x_move=readTwoBytesSigned(fis); //N-Offset <> colon offset <> x_move
            
            hits=readFourBytes(fis);
            misses=readFourBytes(fis);
            
            jumps.add(new RelativeJump(x_move,y_move,hits,misses));
        }
        return jumps;
    }
    
    
    private static Pattern[] readPatterns(FileInputStream fis, int numPatterns) throws IOException {
        Pattern[] patterns=new Pattern[numPatterns];
        
        int pid, numOcc, length;
        
        //Read numPatterns patterns according to specification
        for (int i=0; i<numPatterns; i++){
            pid=fis.read(); //Read Pattern-ID
            
            numOcc=readFourBytes(fis); //Number of Occurences
            length=readTwoBytes(fis); //Number of relative jumps of the pattern
            
            patterns[i]=new Pattern(pid, numOcc, length);
            patterns[i].setJumps(readRelativeJumps(fis,length));
        }
        return patterns;
    }
    
    private static Sequence[] readSequences(FileInputStream fis, int numSequences, Pattern[] patterns) throws IOException {
        Sequence[] sequences=new Sequence[numSequences];
        
        int pid, numOcc, repetitions, nextAccessMOffset, nextAccessNOffset, nextPID;
        
        //Read numSequences sequences according to specification
        for (int i=0; i<numSequences; i++){
            
            pid=fis.read();
            numOcc=readFourBytes(fis); //Number of occurences
            repetitions=readTwoBytes(fis);
            nextAccessMOffset=readTwoBytesSigned(fis);
            nextAccessNOffset=readTwoBytesSigned(fis);
            
            sequences[i]=new Sequence(find(pid, patterns), numOcc, repetitions, nextAccessMOffset, nextAccessNOffset);
            nextPID=fis.read(); //ID of next pattern
            
            //Link to the pattern specified by nextPID
            sequences[i].setNextPattern(nextPID==0xff?null:find(nextPID, patterns));
        }
        return sequences;
    }

    
    //Search for pattern with specified Pattern-ID
    private static Pattern find(int pid, Pattern[] patterns){
        
        for(int i=0; i<patterns.length; i++){
            if (patterns[i].pid==pid) return patterns[i];
        }
        return null;
    }
    
    
    //Read c-style string
    private static String readName(FileInputStream fis) throws IOException {
        String s="";
        int b=fis.read();
        int i=0;
        while(b!=0&&b!=-1){
            s+=(char) b;
            b=fis.read();
            i++;
        }
        if(b==-1) throw new IOException("Error while reading Matrix Name!"+i);
        return s;
    }
}
