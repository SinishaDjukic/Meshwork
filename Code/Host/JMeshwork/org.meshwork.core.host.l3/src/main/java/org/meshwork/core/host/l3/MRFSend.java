package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;
import org.meshwork.core.util.Printer;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MRFSend extends AbstractMessage implements Constants {

    public byte dst;
    public byte port;
    public byte datalen;
    public byte[] data;

    public MRFSend(byte seq) {
        super(MSGCODE_RFSEND, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MRFSend: Dst=");writer.print(dst);
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
        MRFSend result = new MRFSend(msg.seq);
        result.dst = msg.data[0];
        result.port = msg.data[1];
        result.datalen = msg.data[2];
        result.data = new byte[result.datalen];
        if ( result.datalen > 0 )
            System.arraycopy(msg.data, 3, result.data, 0, result.datalen);
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.code = getCode();
        msg.data = new byte[3 + datalen];
        msg.data[0] = dst;
        msg.data[1] = port;
        msg.data[2] = datalen;
        if ( datalen > 0 )
            System.arraycopy(data, 0, msg.data, 3, datalen);
        msg.len = (byte) (msg.data.length + 1);
    }
}
