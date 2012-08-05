package data;

/**
 *
 * @author Simon Wimmer
 */
public class MatrixHeader {

    private int xSize, ySize, numLoadJumps, numStoreJumps, dataType, mibAddr;
    private int numPatterns, numSequences;
    private int storeHits, storeMisses, loadHits, loadMisses;
    private long addr;
    private String name;

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
     * @return the dataType
     */
    public int getDataType() {
        return dataType;
    }

    /**
     * @param dataType the dataType to set
     */
    public void setDataType(int dataType) {
        this.dataType = dataType;
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

    /**
     * @return the name
     */
    public String getName() {
        return name;
    }

    /**
     * @param name the name to set
     */
    public void setName(String name) {
        this.name = name;
    }

    public void printInfo() {
        System.out.println("Header Info for " + name);
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
