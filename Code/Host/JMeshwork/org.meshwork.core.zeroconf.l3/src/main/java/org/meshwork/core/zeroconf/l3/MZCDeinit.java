package org.meshwork.core.zeroconf.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MZCDeinit extends AbstractMessage implements Constants {

    public MZCDeinit(byte seq) {
        super(seq, ZC_CODE, ZC_SUBCODE_ZCDEINIT);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MZCDeinit");
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MZCDeinit result = new MZCDeinit(msg.seq);
        return result;
    }

    @Override
    public void serializeImpl(MessageData msg) {
    }
}
