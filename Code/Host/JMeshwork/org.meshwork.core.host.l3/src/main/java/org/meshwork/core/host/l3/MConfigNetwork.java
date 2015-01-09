package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MConfigNetwork extends AbstractMessage implements Constants {

    public byte channel;
    public short nwkid;
    public byte nodeid;
    public byte keylen;
    public byte[] key;

    public MConfigNetwork(byte seq) {
        super(seq, NS_CODE, NS_SUBCODE_CFGNWK);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MConfigNetwork: Channel=");writer.print(channel);
        writer.print(", NwkID=");writer.print(nwkid);
        writer.print(", NodeID=");writer.print(nodeid);
        writer.print(", KeyLen=");writer.print(keylen);
        if ( key != null ) {
            writer.print(", Key=");writer.print(new String(key));
        }
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MConfigNetwork result = new MConfigNetwork(msg.seq);
        result.channel = msg.data[0];
        result.nwkid = (short) (( msg.data[1] << 8 ) & 0xFF |
                                ( msg.data[2] << 0 ) & 0xFF);
        result.nodeid = msg.data[3];
        result.keylen = msg.data[4];
        result.key = result.keylen < 1 ? null : new byte[result.keylen];
        System.arraycopy(msg.data, 5, result.key, 0, result.keylen);
        return result;
    }

    @Override
    public void serializeImpl(MessageData msg) {
        msg.data = new byte[5 + keylen];
        msg.data[0] = channel;
        msg.data[1] = (byte) (( nwkid >> 8 ) & 0xFF);
        msg.data[2] = (byte) (( nwkid >> 0 ) & 0xFF);
        msg.data[3] = nodeid;
        msg.data[4] = keylen;
        if ( keylen >= 1 )
            System.arraycopy(key, 0, msg.data, 5, keylen);
    }
}
