package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;
import org.meshwork.core.util.Printer;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MRFReceiveACK extends AbstractMessage implements Constants {

    public byte datalen;
    public byte[] data;

    public MRFReceiveACK(byte seq) {
        super(MSGCODE_RFRECVACK, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MRFReceiveACK: DataLen=");writer.print(datalen);
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
        MRFReceiveACK result = new MRFReceiveACK(msg.seq);
        result.datalen = msg.data[0];
        result.data = new byte[result.datalen];
        System.arraycopy(msg.data, 1, result.data, 0, result.datalen);
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.code = getCode();
        msg.data = new byte[1 + datalen];
        msg.data[0] = datalen;
        if ( data != null )
            System.arraycopy(data, 0, msg.data, 1, datalen);
        msg.len = (byte) (msg.data.length + 1);
    }
}
