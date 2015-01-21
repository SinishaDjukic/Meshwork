package org.meshwork.core;

/**
 * Created by Sinisha Djukic on 20.1.2015.
 */
public interface SerialMessageConstants {
    
    public static final byte MAX_SERIALMSG_LEN 			= 32;

    //0-31: Reserved general
    //32-255: Code specific
    public static final byte SM_MESSAGE_UNKNOWN			= 0;
    public static final byte SM_MESSAGE_PROCESSED		= 1;
    public static final byte SM_MESSAGE_NONE			= 2;
    public static final byte SM_MESSAGE_ERROR			= 3;

    //0-31: Reserved general
    //32-255: Code specific
    public static final byte SM_SUBCODE_OK 				= 0;
    public static final byte SM_SUBCODE_NOK 			= 1;
    public static final byte SM_SUBCODE_UNKNOWN 		= 2;

    //0-31: Reserved general
    //32-255: Code/sub-code specific
    public static final byte SM_NOK_GENERAL 			= 0;
    public static final byte SM_NOK_INSUFFICIENT_DATA 	= 1;
    public static final byte SM_NOK_TOO_LONG_DATA 		= 2;
    public static final byte SM_NOK_ILLEGAL_STATE 		= 3;
    public static final byte SM_NOK_SEQUENCE_MISMATCH	= 4;

    //Default message response timeout (ms)
    public static final short TIMEOUT_RESPONSE 			= 5000;

}
