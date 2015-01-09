package org.meshwork.core.zeroconf.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MZCRepRes extends AbstractMessage implements Constants {

    public byte reportNodeid;
    public byte reportFlags;

    public MZCRepRes(byte seq) {
        super(seq, ZC_CODE, ZC_SUBCODE_ZCREPRES);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MZCRepRes: ReportNodeID=");writer.print(reportNodeid);
        writer.print(", RepFlags=");writer.print(reportFlags);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MZCRepRes result = new MZCRepRes(msg.seq);
        result.reportNodeid = msg.data[0];
        result.reportFlags = msg.data[1];
        return result;
    }

    @Override
    public void serializeImpl(MessageData msg) {
        msg.data = new byte[2];
        msg.data[0] = reportNodeid;
        msg.data[1] = reportFlags;
    }
}
