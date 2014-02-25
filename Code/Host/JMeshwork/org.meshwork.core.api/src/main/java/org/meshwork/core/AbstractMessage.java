package org.meshwork.core;

import org.meshwork.core.util.Printable;

import java.io.IOException;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public abstract class AbstractMessage implements Printable {

    public byte seq;
    protected byte code;

    public AbstractMessage(byte code, byte seq) {
        this.code = code;
        this.seq = seq;
    }

    public String toString() {
        StringBuffer sb = new StringBuffer(getClass().getName());
        sb.append(": Seq=").append(seq).append(", Code=").append(code);
        return sb.toString();
    }

    //create a new instance with filled data based on the message
    public abstract AbstractMessage deserialize(MessageData msg) throws IOException;

    //serialize into a data object ready for sending
    public abstract void serialize(MessageData msg);

    public final byte getCode() {
        return code;
    }

}
