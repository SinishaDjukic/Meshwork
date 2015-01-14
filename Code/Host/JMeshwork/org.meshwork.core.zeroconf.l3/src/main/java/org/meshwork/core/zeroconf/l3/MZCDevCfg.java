package org.meshwork.core.zeroconf.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MZCDevCfg extends AbstractMessage implements Constants {

    public byte nwkcaps;
    public byte delivery;

    public MZCDevCfg(byte seq) {
        super(seq, ZC_CODE, ZC_SUBCODE_ZCDEVCFG);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MZCDevCfg: NwkCaps=");writer.print(nwkcaps);
        writer.print(", Delivery=");writer.print(delivery);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MZCDevCfg result = new MZCDevCfg(msg.seq);
        result.nwkcaps = msg.data[0];
        result.delivery = msg.data[1];
        return result;
    }

    @Override
    public void serializeImpl(MessageData msg) {
        msg.data = new byte[2];
        msg.data[0] = nwkcaps;
        msg.data[1] = delivery;
    }
}
