package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MNOK extends AbstractMessage implements Constants {

    public byte error;

    public MNOK(byte seq) {
        super(MSGCODE_NOK, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MNOK: Error=");writer.print(error);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MNOK result = new MNOK(msg.seq);
        result.error = msg.data[0];
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.len = 1+1;
        msg.code = getCode();
        msg.data = new byte[] { error };
    }
}
