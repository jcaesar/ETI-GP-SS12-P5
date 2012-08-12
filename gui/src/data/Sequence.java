package data;

/**
 *
 * @author Simon Wimmer 
 * Represents a Sequence as specified in the
 * Dataformat-Documentation on
 * https://github.com/bucjac/ETI-GP-SS12-P5/wiki/Dateiformat
 */
public class Sequence {

    private Pattern pattern;
    private int numOccurrences;
    private int numRepetitions;
    private int nextAccessMOffset;
    private int nextAccessNOffset;
    private Pattern nextPattern;

    public Sequence(Pattern pattern,
            int numOccurrences,
            int numRepetitions,
            int nextAccessMOffset,
            int nextAccessNOffset) {
        this.pattern = pattern;
        this.numOccurrences = numOccurrences;
        this.numRepetitions = numRepetitions;
        this.nextAccessMOffset = nextAccessMOffset;
        this.nextAccessNOffset = nextAccessNOffset;
    }

    @Override
    public String toString() {
        return "Pattern:\n" + pattern + "\n" + "numOccurrences: " + numOccurrences
                + "\nnumRepetitions: " + numRepetitions
                + "\nnextAccessMOffset: " + nextAccessMOffset
                + "\nnextAccessNOffset: " + nextAccessNOffset
                + "\nnextPattern: " + (nextPattern == null ? "No Pattern found." : "" + nextPattern.pid);
    }

    /**
     * @return the numOccurrences
     */
    public int getNumOccurrences() {
        return numOccurrences;
    }

    /**
     * @param numOccurrences the numOccurrences to set
     */
    public void setNumOccurrences(int numOccurrences) {
        this.numOccurrences = numOccurrences;
    }

    /**
     * @return the numRepetitions
     */
    public int getNumRepetitions() {
        return numRepetitions;
    }

    /**
     * @param numRepetitions the numRepetitions to set
     */
    public void setNumRepetitions(int numRepetitions) {
        this.numRepetitions = numRepetitions;
    }

    /**
     * @return the nextAccessMOffset
     */
    public int getNextAccessMOffset() {
        return nextAccessMOffset;
    }

    /**
     * @param nextAccessMOffset the nextAccessMOffset to set
     */
    public void setNextAccessMOffset(int nextAccessMOffset) {
        this.nextAccessMOffset = nextAccessMOffset;
    }

    /**
     * @return the nextAccessNOffset
     */
    public int getNextAccessNOffset() {
        return nextAccessNOffset;
    }

    /**
     * @param nextAccessNOffset the nextAccessNOffset to set
     */
    public void setNextAccessNOffset(int nextAccessNOffset) {
        this.nextAccessNOffset = nextAccessNOffset;
    }

    /**
     * @return the nextPattern
     */
    public Pattern getNextPattern() {
        return nextPattern;
    }

    /**
     * @param nextPattern the nextPattern to set
     */
    public void setNextPattern(Pattern nextPattern) {
        this.nextPattern = nextPattern;
    }

    /**
     * @return the pattern
     */
    public Pattern getPattern() {
        return pattern;
    }

    /**
     * @param pattern the pattern to set
     */
    public void setPattern(Pattern pattern) {
        this.pattern = pattern;
    }

    @Override
    public boolean equals(Object anObject) {
        if (anObject instanceof Sequence) {
            Sequence s = (Sequence) anObject;
            return s.getPattern().pid == getPattern().pid
                    && //s.nextAccessMOffset==nextAccessMOffset&&s.nextAccessNOffset==nextAccessNOffset&&
                    s.nextPattern.pid == nextPattern.pid && s.numRepetitions == numRepetitions;
        }
        return false;
    }
}
