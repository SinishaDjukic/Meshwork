package org.meshwork.core.zeroconf.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MZCCfgRep extends AbstractMessage implements Constants {

    public byte reportNodeid;
    public byte reportFlags;

    public MZCCfgRep(byte seq) {
        super(MSGCODE_ZCCFGNWK, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MZCCfgRep: ReportNodeID=");writer.print(reportNodeid);
        writer.print(", RepFlags=");writer.print(reportFlags);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MZCCfgRep result = new MZCCfgRep(msg.seq);
        result.reportNodeid = msg.data[0];
        result.reportFlags = msg.data[1];
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.code = getCode();
        msg.data = new byte[2];
        msg.data[0] = reportNodeid;
        msg.data[1] = reportFlags;
        msg.len = (byte) (msg.data.length + 1);
    }
}
