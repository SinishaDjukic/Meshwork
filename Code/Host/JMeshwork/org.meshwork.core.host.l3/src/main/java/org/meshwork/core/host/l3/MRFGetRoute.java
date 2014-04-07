package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MRFGetRoute extends AbstractMessage implements Constants {

    public byte dst;
    public byte index;

    public MRFGetRoute(byte seq) {
        super(MSGCODE_RFGETROUTE, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MRFGetRoute: Dst=");writer.print(dst);
        writer.print(", Index=");writer.print(index);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MRFGetRoute result = new MRFGetRoute(msg.seq);
        result.dst = msg.data[0];
        result.index = msg.data[1];
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.code = getCode();
        msg.data = new byte[2];
        msg.data[0] = dst;
        msg.data[1] = index;
        msg.len = (byte) (msg.data.length + 1);
    }
}
