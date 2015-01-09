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
        super(seq, NS_CODE, NS_SUBCODE_RFSENDACK);
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
        result.datalen = msg.data[0];
        result.data = new byte[result.datalen];
        System.arraycopy(msg.data, 1, result.data, 0, result.datalen);
        return result;
    }

    @Override
    public void serializeImpl(MessageData msg) {
        msg.data = new byte[datalen];
        msg.data[0] = datalen;
        System.arraycopy(data, 0, msg.data, 1, datalen);
    }
}
