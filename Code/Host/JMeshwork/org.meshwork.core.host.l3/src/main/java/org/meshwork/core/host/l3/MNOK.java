package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;
import org.meshwork.core.SerialMessageConstants;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MNOK extends AbstractMessage implements Constants {

    public static final String GENERAL_ERROR        = "GENERAL ERROR";
    public static final String INSUFFICIENT_DATA    = "INSUFFICIENT DATA";
    public static final String TOO_LONG_DATA        = "TOO LONG DATA";
    public static final String ILLEGAL_STATE        = "ILLEGAL STATE";
    public static final String RECEIVE_ERROR        = "RECEIVE ERROR";
    public static final String SEND_ERROR           = "SEND ERROR";
    public static final String BROADCAST_ERROR      = "BROADCAST ERROR";
    public static final String KEY_TOO_LONG         = "KEY TOO LONG";
    public static final String SEQUENCE_MISMATCH    = "SEQUENCE MISMATCH";
    public byte error;

    public MNOK(byte seq) {
        super(seq, NS_CODE, SerialMessageConstants.SM_SUBCODE_NOK);
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MNOK: Error=");writer.print(error);

        String errorText = null;
        switch ( error ) {
            case SerialMessageConstants.SM_NOK_GENERAL: errorText = GENERAL_ERROR;break;
            case SerialMessageConstants.SM_NOK_INSUFFICIENT_DATA: errorText = INSUFFICIENT_DATA;break;
            case SerialMessageConstants.SM_NOK_TOO_LONG_DATA: errorText = TOO_LONG_DATA;break;
            case SerialMessageConstants.SM_NOK_ILLEGAL_STATE: errorText = ILLEGAL_STATE;break;
            case NS_NOK_RECV: errorText = RECEIVE_ERROR;break;
            case NS_NOK_SEND: errorText = SEND_ERROR;break;
            case NS_NOK_BCAST: errorText = BROADCAST_ERROR;break;
            case NS_NOK_KEY_TOO_LONG: errorText = KEY_TOO_LONG;break;
            case SerialMessageConstants.SM_NOK_SEQUENCE_MISMATCH: errorText = SEQUENCE_MISMATCH;break;
        }
        if ( errorText != null ) {
            writer.print(" ("+errorText+")");
        }
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MNOK result = new MNOK(msg.seq);
        result.error = msg.data[0];
        return result;
    }

    @Override
    public void serializeImpl(MessageData msg) {
        msg.data = new byte[] { error };
    }
}
