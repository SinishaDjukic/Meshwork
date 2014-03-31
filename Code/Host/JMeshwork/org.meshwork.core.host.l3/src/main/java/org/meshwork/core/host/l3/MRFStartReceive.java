package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MRFStartReceive extends AbstractMessage implements Constants {

    public long timeout;

    public MRFStartReceive(byte seq) {
        super(MSGCODE_RFSTARTRECV, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MRFStartReceive: Timeout=");writer.print(timeout);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MRFStartReceive result = new MRFStartReceive(msg.seq);
        result.timeout = (long) (( msg.data[0] << 24 ) & 0xFF |
                                 ( msg.data[1] << 16 ) & 0xFF |
                                 ( msg.data[2] << 8  ) & 0xFF |
                                 ( msg.data[3] << 0  ) & 0xFF);
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.code = getCode();
        msg.data = new byte[4];
        msg.data[0] = (byte) (( timeout >> 24 ) & 0xFF);
        msg.data[1] = (byte) (( timeout >> 16 ) & 0xFF);
        msg.data[2] = (byte) (( timeout >> 8  ) & 0xFF);
        msg.data[3] = (byte) (( timeout >> 0  ) & 0xFF);
        msg.len = (byte) (msg.data.length + 1);
    }
}
