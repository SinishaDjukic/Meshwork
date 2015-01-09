package org.meshwork.core;

import org.meshwork.core.util.Printable;
import org.meshwork.core.util.Printer;

import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MessageData implements Printable {
    public byte len;
    public byte seq;
    public byte code;
    public byte subCode;
    public byte data[];

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("Len=");writer.print(len);
        writer.print(", Seq=");writer.print(seq);
        writer.print(", Code=");writer.print(code);
        writer.print(", SubCode=");writer.print(subCode);
        if ( len > 3 ) {
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
        sb.append(", Len=").append(len).append(": Seq=").append(seq).append(", Code=").append(code).append(", SubCode=").append(subCode);
        return sb.toString();
    }
}
