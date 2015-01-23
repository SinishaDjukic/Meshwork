package org.meshwork.core.zeroconf.l3;

/**
 * Created by Sinisha Djukic on 14-2-11.
 */
public interface Constants {

    public static final byte MAX_SERIAL_LEN 			= 16;

    //ZeroConf Serial Code ID
    public static final byte ZC_CODE 					= 1;

    //32-255: Code specific
    public static final byte ZC_SUBCODE_ZCINIT 			= 41;
    public static final byte ZC_SUBCODE_ZCDEINIT 	    = 42;
    public static final byte ZC_SUBCODE_ZCDEVREQ        = 43;
    public static final byte ZC_SUBCODE_ZCDEVRES        = 44;
    public static final byte ZC_SUBCODE_ZCDEVCFG        = 45;
    public static final byte ZC_SUBCODE_ZCNWKREQ        = 46;
    public static final byte ZC_SUBCODE_ZCNWKRES        = 47;
    public static final byte ZC_SUBCODE_ZCNWKCFG        = 48;
    public static final byte ZC_SUBCODE_ZCREPREQ        = 49;
    public static final byte ZC_SUBCODE_ZCREPRES 		= 50;
    public static final byte ZC_SUBCODE_ZCREPCFG 		= 51;
    public static final byte ZC_SUBCODE_ZCSERIALREQ     = 52;
    public static final byte ZC_SUBCODE_ZCSERIALRES		= 53;
    public static final byte ZC_SUBCODE_ZCSERIALCFG		= 54;
    public static final byte ZC_SUBCODE_ZCFACTORYRESET	= (byte) 255;

    //32-255: Code/sub-code specific
    public static final byte ZC_NOK_KEY_TOO_LONG 		= 32;
    public static final byte ZC_NOK_SERIAL_TOO_LONG 	= 33;

    //flags for configuring reporting
    public static final byte MASK_REPORT_NWK_ADD_REMOVE		= 1 << 0;
    public static final byte MASK_REPORT_DISCRETE_CHANGE	= 1 << 1;
    public static final byte MASK_REPORT_THRESHOLD_CHANGE	= 1 << 2;


    //TODO same as in host.l3.Constants - pull to a common place
    /** First possible node ID. */
    public static final byte MIN_NODE_ID 	= 1;
    /** Last possible node ID. */
    public static final byte MAX_NODE_ID 	= (byte) 254;
    /** Maximum node count in the network. */
    public static final byte MAX_NODE_COUNT = MAX_NODE_ID - MIN_NODE_ID;

    /** Maximum length of a network key. */
    public static final byte MAX_NETWORK_KEY_LEN 	= 8;

}
