package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MConfigBasic extends AbstractMessage implements Constants {

    public byte nwkcaps;
    public byte delivery;
    public byte retry;
    public byte keylen;
    public byte[] key;

    public MConfigBasic(byte seq) {
        super(MSGCODE_CFGBASIC, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MConfigBasic: NwkCaps=");writer.print(nwkcaps);
        writer.print(", Delivery=");writer.print(delivery);
        writer.print(", Retry=");writer.print(retry);
        writer.print(", KeyLen=");writer.print(keylen);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MConfigBasic result = new MConfigBasic(msg.seq);
        result.nwkcaps = msg.data[0];
        result.delivery = msg.data[1];
        result.retry = msg.data[2];
        result.keylen = msg.data[3];
        result.key = keylen < 1 ? null : new byte[keylen];
        System.arraycopy(msg.data, 4, result.key, 0, keylen);
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.len = 5;
        msg.code = getCode();
        msg.data = new byte[msg.len - 1 + keylen];
        msg.data[0] = nwkcaps;
        msg.data[1] = delivery;
        msg.data[2] = retry;
        msg.data[3] = keylen;
        if ( keylen >= 1 )
            System.arraycopy(key, 0, msg.data, 4, keylen);
    }
}
