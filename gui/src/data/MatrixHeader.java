package data;

/**
 *
 * @author Simon Wimmer 
 * Represents a Matrix-Header as specified in the
 * Dataformat-Documentation on
 * https://github.com/bucjac/ETI-GP-SS12-P5/wiki/Dateiformat
 */
public class MatrixHeader {

    private int xSize, ySize, numLoadJumps, numStoreJumps, mibAddr;
    private int numPatterns, numSequences;
    private int storeHits, storeMisses, loadHits, loadMisses;
    private long addr;

    /**
     * @return the xSize
     */
    public int getxSize() {
        return xSize;
    }

    /**
     * @param xSize the xSize to set
     */
    public void setxSize(int xSize) {
        this.xSize = xSize;
    }

    /**
     * @return the ySize
     */
    public int getySize() {
        return ySize;
    }

    /**
     * @param ySize the ySize to set
     */
    public void setySize(int ySize) {
        this.ySize = ySize;
    }

    /**
     * @return the numJumps
     */
    public int getNumLoadJumps() {
        return numLoadJumps;
    }

    /**
     * @param numJumps the numJumps to set
     */
    public void setNumLoadJumps(int numJumps) {
        this.numLoadJumps = numJumps;
    }

    /**
     * @return the numJumps
     */
    public int getNumStoreJumps() {
        return numStoreJumps;
    }

    /**
     * @param numJumps the numJumps to set
     */
    public void setNumStoreJumps(int numJumps) {
        this.numStoreJumps = numJumps;
    }

    /**
     * @return the mibAddr
     */
    public int getMibAddr() {
        return mibAddr;
    }

    /**
     * @param mibAddr the mibAddr to set
     */
    public void setMibAddr(int mibAddr) {
        this.mibAddr = mibAddr;
    }

    /**
     * @return the addr
     */
    public long getAddr() {
        return addr;
    }

    /**
     * @param addr the addr to set
     */
    public void setAddr(long addr) {
        this.addr = addr;
    }

    public void printInfo() {
        System.out.println("Header Info:");
        System.out.println("X " + xSize);
        System.out.println("Y " + ySize);
        System.out.println("Num Load Jumps " + numLoadJumps);
        System.out.println("Num Store Jumps " + numStoreJumps);
        System.out.println("Num Patterns " + numPatterns);
        System.out.println("Num Sequences " + numSequences);
        System.out.println("MIBADDR " + Integer.toHexString(mibAddr));
    }

    /**
     * @return the storeHits
     */
    public int getStoreHits() {
        return storeHits;
    }

    /**
     * @param storeHits the storeHits to set
     */
    public void setStoreHits(int storeHits) {
        this.storeHits = storeHits;
    }

    /**
     * @return the storeMisses
     */
    public int getStoreMisses() {
        return storeMisses;
    }

    /**
     * @param storeMisses the storeMisses to set
     */
    public void setStoreMisses(int storeMisses) {
        this.storeMisses = storeMisses;
    }

    /**
     * @return the loadHits
     */
    public int getLoadHits() {
        return loadHits;
    }

    /**
     * @param loadHits the loadHits to set
     */
    public void setLoadHits(int loadHits) {
        this.loadHits = loadHits;
    }

    /**
     * @return the loadMisses
     */
    public int getLoadMisses() {
        return loadMisses;
    }

    /**
     * @param loadMisses the loadMisses to set
     */
    public void setLoadMisses(int loadMisses) {
        this.loadMisses = loadMisses;
    }

    /**
     * @return the numPatterns
     */
    public int getNumPatterns() {
        return numPatterns;
    }

    /**
     * @param numPatterns the numPatterns to set
     */
    public void setNumPatterns(int numPatterns) {
        this.numPatterns = numPatterns;
    }

    /**
     * @return the numSequences
     */
    public int getNumSequences() {
        return numSequences;
    }

    /**
     * @param numSequences the numSequences to set
     */
    public void setNumSequences(int numSequences) {
        this.numSequences = numSequences;
    }
}
