package org.meshwork.core.util;

import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.HashMap;

/**
 * Created by Sinisha Djukic on 14-10-4.
 */
public class IndentedPrintWriter extends PrintWriter {

    //Thread.hashCode() -> initial indent
    protected HashMap<Integer, Integer> indent;
    //Prefix for each new line
    protected String linePrefix;
    //Prefix in a char[] form for efficiency
    protected char[] linePrefixBuf;
    //Prefix len
    protected int linePrefixBufLen;

    public IndentedPrintWriter(OutputStream out, boolean autoFlush, String prefix) {
        super(out, autoFlush);
        setLinePrefix(prefix);
        indent = new HashMap<Integer, Integer>();
    }

    public void initThread(Thread t, int adjust) {
        StackTraceElement[] se = t.getStackTrace();
        indent.put(t.hashCode(), adjust + (se != null ? se.length : 0));
    }

    public void deinitThread(Thread t) {
        indent.remove(t.hashCode());
    }

    public String getLinePrefix() {
        return linePrefix;
    }

    public void setLinePrefix(String linePrefix) {
        this.linePrefix = linePrefix;
        linePrefixBuf = linePrefix == null ? null : linePrefix.toCharArray();
        linePrefixBufLen = linePrefixBuf == null ? 0 : linePrefixBuf.length;
    }

    @Override
    public void write(char[] buf, int off, int len) {
        System.out.println("SHOULD BE HERE!");
        if ( linePrefixBuf == null ) {
            super.write(buf, off, len);
        } else {
            int pos = off;
            int posMax = off + len;
            int lastRN = -1;

            while (pos < posMax) {
                if ( buf[pos] == '\r' || buf[pos] == 'n' )
                    lastRN = pos;
                pos ++;
            }
            if ( lastRN == -1 ) {
                super.write(buf, off, len);
            } else {
                super.write(buf, lastRN, lastRN - off + 1);
                int indents = getIndents(Thread.currentThread());
                System.out.println("\t INDENTS: "+indents);
                for ( int i = 0; i < indents; i ++ )
                    super.write(linePrefixBuf, 0, linePrefixBufLen);
                if ( lastRN < posMax )
                    super.write(buf, lastRN + 1, lastRN - off);
            }
        }
    }

    private int getIndents(Thread thread) {
        Integer adjust = indent.get(thread.hashCode());
        StackTraceElement[] se = thread.getStackTrace();
        return (adjust == null ? 0 : adjust) + (se != null ? se.length: 0);
    }
}