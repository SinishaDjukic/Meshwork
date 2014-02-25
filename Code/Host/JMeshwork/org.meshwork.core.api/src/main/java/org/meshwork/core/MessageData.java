package org.meshwork.core;

import org.meshwork.core.util.Printable;
import org.meshwork.core.util.Printer;

import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MessageData implements Printable {
    public byte seq;
    public byte len;
    public byte code;
    public byte data[];

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("Seq=");writer.print(seq);
        writer.print(", Len=");writer.print(len);
        writer.print(", Code=");writer.print(code);
        if ( len >= 2 ) {
            writer.print(", Data={");
            if ( data != null && data.length == len -1 ) {
                Printer.printHex(writer, data, -1, 8, rowPrefix, rowSuffix, separator);
            } else
                writer.print("ERROR");
            writer.print("}");
        };
    }
    public String toString() {
        StringBuffer sb = new StringBuffer(getClass().getName());
        sb.append(": Seq=").append(seq).append(", Len=").append(len).append(", Code=").append(code);
        return sb.toString();
    }
}
