package org.meshwork.core.zeroconf.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MOK extends AbstractMessage implements Constants {

    public MOK(byte seq) {
        super(seq, ZC_CODE, ZC_SUBCODE_OK);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MOK");
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MOK result = new MOK(seq);
        return result;
    }

    @Override
    public void serializeImpl(MessageData msg) {
    }
}
