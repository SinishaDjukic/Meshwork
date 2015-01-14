package org.meshwork.core;

import org.meshwork.core.util.Printable;

import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public abstract class AbstractMessage implements Printable {

    public byte seq;
    protected byte code;
    protected byte subCode;

    public AbstractMessage(byte seq, byte code, byte subCode) {
        this.seq = seq;
        this.code = code;
        this.subCode = subCode;
    }

    public String toString() {
        StringBuffer sb = new StringBuffer(getClass().getName());
        sb.append(": Seq=").append(seq).append(", Code=").append(code).append(", SubCode=").append(subCode);
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        toString(pw, null, null, null);
        pw.flush();
        sw.flush();
        sb.append(", ").append(sw);
        return sb.toString();
    }

    //create a new instance with filled data based on the message
    public abstract AbstractMessage deserialize(MessageData msg) throws IOException;

    //abstract method to be implemented in subclasses
    public abstract void serializeImpl(MessageData msg);

    //serialize into a data object ready for sending
    public final void serialize(MessageData msg) {
        msg.data = null;
        serializeImpl(msg);
        //default implementation fills in mandatory fields
        msg.seq = seq;
        msg.code = getCode();
        msg.subCode = getSubCode();
        msg.len = (byte) (3 + (msg.data == null ? 0 : msg.data.length));
    }

    public final byte getCode() {
        return code;
    }

    public final byte getSubCode() {
        return subCode;
    }

}
