package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;
import org.meshwork.core.util.Printer;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MRFReceive extends AbstractMessage implements Constants {

    public byte src;
    public byte port;
    public byte datalen;
    public byte[] data;

    public MRFReceive(byte seq) {
        super(MSGCODE_RFRECV, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MRFReceive: Src=");writer.print(src);
        writer.print(", Port=");writer.print(port);
        writer.print(", DataLen=");writer.print(datalen);
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
        MRFReceive result = new MRFReceive(msg.seq);
        result.src = msg.data[0];
        result.port = msg.data[1];
        result.datalen = msg.data[2];
        result.data = new byte[result.datalen];
        System.arraycopy(msg.data, 3, result.data, 0, result.datalen);
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.len = (byte) (1+3+datalen);
        msg.code = getCode();
        msg.data = new byte[3+datalen];
        msg.data[0] = src;
        msg.data[1] = port;
        msg.data[2] = datalen;
        System.arraycopy(data, 0, msg.data, 3, datalen);
    }
}
