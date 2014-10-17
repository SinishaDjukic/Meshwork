package org.meshwork.core.zeroconf.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MZCIDRes extends AbstractMessage implements Constants {

    public byte nwkcaps;
    public byte delivery;
    public byte sernumlen;
    public byte[] sernum;

    public MZCIDRes(byte seq) {
        super(MSGCODE_ZCIDRES, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MZCIDRes: NwkCaps=");writer.print(nwkcaps);
        writer.print(", Delivery=");writer.print(delivery);
        writer.print(", SerNumLen=");writer.print(sernumlen);
        if ( sernum != null ) {
            writer.print(", SerNum=");writer.print(new String(sernum));
        }
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MZCIDRes result = new MZCIDRes(seq);
        result.nwkcaps = msg.data[0];
        result.delivery = msg.data[1];
        result.sernumlen = msg.data[2];
        result.sernum = result.sernumlen < 1 ? null : new byte[result.sernumlen];
        System.arraycopy(msg.data, 3, result.sernum, 0, result.sernumlen);
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.code = getCode();
        msg.data = new byte[3 + sernumlen];
        msg.data[0] = nwkcaps;
        msg.data[1] = delivery;
        msg.data[2] = sernumlen;
        if ( sernumlen >= 1 )
            System.arraycopy(sernum, 0, msg.data, 3, sernumlen);
        msg.len = (byte) (msg.data.length + 1);
    }
}
