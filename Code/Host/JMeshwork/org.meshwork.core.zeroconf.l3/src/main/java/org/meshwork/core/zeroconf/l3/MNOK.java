package org.meshwork.core.zeroconf.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MNOK extends AbstractMessage implements Constants {

    public byte error;

    public MNOK(byte seq) {
        super(MSGCODE_NOK, seq);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MNOK: Error=");writer.print(error);

        String errorText = null;
        //TODO put error strings into a hash map
        switch ( error ) {
            case ERROR_GENERAL: errorText = "(GENERAL ERROR)";break;
            case ERROR_INSUFFICIENT_DATA: errorText = "(INSUFFICIENT DATA)";break;
            case ERROR_TOO_LONG_DATA: errorText = "(TOO LONG DATA)";break;
            case ERROR_ILLEGAL_STATE: errorText = "(ILLEGAL STATE)";break;
            case ERROR_KEY_TOO_LONG: errorText = "(KEY TOO LONG)";break;
            case ERROR_SEQUENCE_MISMATCH: errorText = "(SEQUENCE MISMATCH)";break;
        }
        if ( errorText != null ) {
            writer.print(" ");
            writer.print(errorText);
        }
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MNOK result = new MNOK(msg.seq);
        result.error = msg.data[0];
        return result;
    }

    @Override
    public void serialize(MessageData msg) {
        msg.seq = seq;
        msg.code = getCode();
        msg.data = new byte[] { error };
        msg.len = (byte) (msg.data.length + 1);
    }
}
