package aquila4j;

import java.math.BigInteger;

public class Helper {

    public static BigInteger intFactorial(BigInteger n) {
        return n != null && n.compareTo(BigInteger.ONE) > 0
            ? n.multiply(intFactorial(n.subtract(BigInteger.ONE)))
            : BigInteger.ONE;
    }

    public static String strDropQuotes(String s) {
        return s != null && s.length() >= 2 && s.startsWith("'") && s.endsWith("'")
            ? s.substring(1, s.length() - 1)
            : s;
    }

    public static boolean strIsNumeric(String s) {
        return s != null && s.matches("[0-9]+");
    }

}
