package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MRFDeinit extends AbstractMessage implements Constants {

    public MRFDeinit(byte seq) {
        super(MSGCODE_RFDEINIT, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MRFDeinit");
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MRFDeinit result = new MRFDeinit(seq);
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.len = 1;
        msg.code = getCode();
        msg.data = null;
    }
}
