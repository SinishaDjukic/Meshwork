package org.meshwork.core.zeroconf.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;
import org.meshwork.core.SerialMessageConstants;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MUnknown extends AbstractMessage implements Constants {

    public MUnknown(byte seq) {
        super(seq, ZC_CODE, SerialMessageConstants.SM_SUBCODE_UNKNOWN);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MUnknown");
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MUnknown result = new MUnknown(msg.seq);
        return result;
    }

    @Override
    public void serializeImpl(MessageData msg) {
    }
}
