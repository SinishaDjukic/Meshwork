package org.meshwork.core.zeroconf.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MZCNwkIDRes extends AbstractMessage implements Constants {

    public byte channel;
    public short nwkid;
    public byte nodeid;

    public MZCNwkIDRes(byte seq) {
        super(MSGCODE_ZCIDRES, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MZCNwkIDRes: Channel=");writer.print(channel);
        writer.print(", NwkID=");writer.print(nwkid);
        writer.print(", NodeID=");writer.print(nodeid);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MZCNwkIDRes result = new MZCNwkIDRes(seq);
        result.channel = msg.data[0];
        result.nwkid = (short) (( msg.data[1] << 8 ) & 0xFF |
                                ( msg.data[2] << 0 ) & 0xFF);
        result.nodeid = msg.data[3];
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.code = getCode();
        msg.data = new byte[4];
        msg.data[0] = channel;
        msg.data[1] = (byte) (( nwkid >> 8 ) & 0xFF);
        msg.data[2] = (byte) (( nwkid >> 0 ) & 0xFF);
        msg.data[4] = nodeid;
        msg.len = (byte) (msg.data.length + 1);
    }
}
