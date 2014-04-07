package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;
import org.meshwork.core.util.Printer;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MRFBroadcast extends AbstractMessage implements Constants {

    public byte port;
    public byte datalen;
    public byte[] data;

    public MRFBroadcast(byte seq) {
        super(MSGCODE_RFBCAST, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MRFBroadcast: Port=");writer.print(port);
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
        MRFBroadcast result = new MRFBroadcast(msg.seq);
        result.port = msg.data[0];
        result.datalen = msg.data[1];
        result.data = new byte[result.datalen];
        System.arraycopy(msg.data, 2, result.data, 0, result.datalen);
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.code = getCode();
        msg.data = new byte[2 + datalen];
        msg.data[0] = port;
        msg.data[1] = datalen;
        if ( datalen > 0)
            System.arraycopy(data, 0, msg.data, 2, datalen);
        msg.len = (byte) (msg.data.length + 1);
    }
}
