package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MUnknown extends AbstractMessage implements Constants {

    public MUnknown(byte seq) {
        super(MSGCODE_UNKNOWN, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MUnknown");
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MUnknown result = new MUnknown(msg.seq);
        result.seq = msg.seq;
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.code = getCode();
        msg.data = null;
        msg.len = 1;
    }
}
