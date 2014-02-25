package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;
import org.meshwork.core.util.Printer;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MRFSendACK extends AbstractMessage implements Constants {

    public byte datalen;
    public byte[] data;

    public MRFSendACK(byte seq) {
        super(MSGCODE_RFSENDACK, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MRFSendACK: DataLen=");writer.print(datalen);
        if ( datalen > 0 ) {
            writer.print(", Data={");
            if ( data != null && data.length == datalen ) {
                Printer.printHex(writer, data, -1, 0, rowPrefix, rowSuffix, separator);
            } else
                writer.print("ERROR");
            writer.print("}");
        };
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MRFSendACK result = new MRFSendACK(msg.seq);
        result.seq = msg.seq;
        result.datalen = msg.data[0];
        result.data = new byte[result.datalen];
        System.arraycopy(msg.data, 1, result.data, 0, result.datalen);
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.len = (byte) (1+1+datalen);
        msg.code = getCode();
        msg.data = new byte[msg.len-1];
        msg.data[0] = datalen;
        System.arraycopy(data, 0, msg.data, 1, datalen);
    }
}
