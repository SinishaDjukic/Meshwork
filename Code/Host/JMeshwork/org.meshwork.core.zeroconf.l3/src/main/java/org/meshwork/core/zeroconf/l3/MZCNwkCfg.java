package org.meshwork.core.zeroconf.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MZCNwkCfg extends AbstractMessage implements Constants {

    public byte channel;
    public short nwkid;
    public byte nodeid;
    public byte keylen;
    public byte[] key;

    public MZCNwkCfg(byte seq) {
        super(MSGCODE_ZCNWKCFG, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MZCNwkCfg: Channel=");writer.print(channel);
        writer.print(", NwkID=");writer.print(nwkid);
        writer.print(", NodeID=");writer.print(nodeid);
        writer.print(", KeyLen=");writer.print(keylen);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MZCNwkCfg result = new MZCNwkCfg(msg.seq);
        result.channel = msg.data[0];
        result.nwkid = (short) (( msg.data[1] << 8 ) & 0xFF |
                                ( msg.data[2] << 0 ) & 0xFF);
        result.nodeid = msg.data[3];
        result.keylen = msg.data[4];
        result.key = result.keylen < 1 ? null : new byte[result.keylen];
        if ( keylen >= 1 )
            System.arraycopy(msg.data, 5, result.key, 0, result.keylen);
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.code = getCode();
        msg.data = new byte[5 + keylen];
        msg.data[0] = channel;
        msg.data[1] = (byte) (( nwkid >> 8 ) & 0xFF);
        msg.data[2] = (byte) (( nwkid >> 0 ) & 0xFF);
        msg.data[3] = nodeid;
        msg.data[4] = keylen;
        if ( keylen >= 1 )
            System.arraycopy(key, 0, msg.data, 5, keylen);
        msg.len = (byte) (msg.data.length + 1);
    }
}
