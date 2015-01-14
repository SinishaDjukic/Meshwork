package org.meshwork.core.zeroconf.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MZCInit extends AbstractMessage implements Constants {

    public MZCInit(byte seq) {
        super(seq, ZC_CODE, ZC_SUBCODE_ZCINIT);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MZCInit");
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MZCInit result = new MZCInit(msg.seq);
        return result;
    }

    @Override
    public void serializeImpl(MessageData msg) {
    }
}
