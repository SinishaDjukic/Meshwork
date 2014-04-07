package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MRFGetRouteCountRes extends AbstractMessage implements Constants {

    public byte count;

    public MRFGetRouteCountRes(byte seq) {
        super(MSGCODE_RFGETROUTECOUNTRES, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MRFGetRouteCountRes: Count=");writer.print(count);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MRFGetRouteCountRes result = new MRFGetRouteCountRes(msg.seq);
        result.count = msg.data[0];
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.code = getCode();
        msg.data = new byte[1];
        msg.data[0] = count;
        msg.len = (byte) (msg.data.length + 1);
    }
}
