package org.meshwork.core.zeroconf.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MZCSerialCfg extends AbstractMessage implements Constants {

    public byte sernumlen;
    public byte[] sernum;

    public MZCSerialCfg(byte seq) {
        super(seq, ZC_CODE, ZC_SUBCODE_ZCSERIALCFG);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MZCSerialCfg: SerNumLen=");writer.print(sernumlen);
        writer.print(", SerNum=");writer.print(sernum);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MZCSerialCfg result = new MZCSerialCfg(msg.seq);
        result.sernumlen = msg.data[0];
        result.sernum = result.sernumlen < 1 ? null : new byte[result.sernumlen];
        System.arraycopy(msg.data, 1, result.sernum, 0, result.sernumlen);
        return result;
    }

    @Override
    public void serializeImpl(MessageData msg) {
        msg.data = new byte[1 + sernumlen];
        msg.data[0] = sernumlen;
        if ( sernumlen >= 1 )
            System.arraycopy(sernum, 0, msg.data, 1, sernumlen);
    }
}
