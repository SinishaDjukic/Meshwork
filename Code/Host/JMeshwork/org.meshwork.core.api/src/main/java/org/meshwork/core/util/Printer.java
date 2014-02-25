package org.meshwork.core.util;

import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-13.
 */
public class Printer {

    public static final synchronized void printHex(PrintWriter writer, byte[] data,
                                      int maxCount, int maxPerRow,
                                      String rowPrefix, String rowSuffix, String separator) {
        int len = data.length;
        int column = 0;
        for ( int i = 0; i < len; i ++ ) {
            if ( i == maxCount ) {
                writer.print("...");
                writer.println(rowSuffix == null ? "" : rowSuffix);
                return;
            }
            writer.print(Integer.toHexString(data[i]));
            if ( i < len - 1 )
                writer.print(separator == null ? ", " : separator);
            if ( ++ column == maxPerRow ) {
                column = 0;
                writer.println(rowSuffix == null ? "" : rowSuffix);
                writer.print(rowPrefix == null ? "" : rowPrefix);
            }
        }
    }

}
