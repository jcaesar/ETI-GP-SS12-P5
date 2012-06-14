package data;

/**
 *
 * @author Simon Wimmer
 */
public class MatrixHeader {

    private int xSize, ySize, numJumps, dataType, mibAddr;
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
    public int getNumJumps() {
        return numJumps;
    }

    /**
     * @param numJumps the numJumps to set
     */
    public void setNumJumps(int numJumps) {
        this.numJumps = numJumps;
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
        System.out.println("Num Jumps " + numJumps);
    }
}
