package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MConfigNetwork extends AbstractMessage implements Constants {

    public short nwkid;
    public byte nodeid;
    public byte channel;

    public MConfigNetwork(byte seq) {
        super(MSGCODE_CFGNWK, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MConfigNetwork: NwkID=");writer.print(nwkid);
        writer.print(", NodeID=");writer.print(nodeid);
        writer.print(", Channel=");writer.print(channel);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MConfigNetwork result = new MConfigNetwork(msg.seq);
        result.nwkid = (short) (( msg.data[0] << 8 ) & 0xFF |
                                ( msg.data[1] << 0 ) & 0xFF);
        result.nodeid = msg.data[2];
        result.channel = msg.data[3];
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.len = 1+4;
        msg.code = getCode();
        msg.data = new byte[msg.len-1];
        msg.data[0] = (byte) (( nwkid >> 8 ) & 0xFF);
        msg.data[1] = (byte) (( nwkid >> 0 ) & 0xFF);
        msg.data[2] = nodeid;
        msg.data[3] = channel;
    }
}
