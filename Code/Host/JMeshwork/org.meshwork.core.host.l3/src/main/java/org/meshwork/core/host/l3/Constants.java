package org.meshwork.core.host.l3;

/**
 * Created by Sinisha Djukic on 14-2-11.
 */
public interface Constants {

    //Network Serial Code ID
    public static final byte NS_CODE 					    = 0;

    //32-255: Code specific
    public static final byte NS_SUBCODE_INTERNAL 			= 32;
    public static final byte NS_SUBCODE_CFGBASIC 			= 41;//0x
    public static final byte NS_SUBCODE_CFGNWK 				= 42;//0x
    public static final byte NS_SUBCODE_RFINIT 				= 43;//0x
    public static final byte NS_SUBCODE_RFDEINIT 			= 44;//0x
    public static final byte NS_SUBCODE_RFRECV 				= 45;//0x
    public static final byte NS_SUBCODE_RFRECVACK 			= 46;//0x
    public static final byte NS_SUBCODE_RFSTARTRECV 		= 47;//0x
    public static final byte NS_SUBCODE_RFSEND 				= 48;//0x
    public static final byte NS_SUBCODE_RFSENDACK 			= 49;//0x
    public static final byte NS_SUBCODE_RFBCAST 			= 50;//0x
    public static final byte NS_SUBCODE_CFGREQUEST			= 51;//0x
    public static final byte NS_SUBCODE_RFROUTEFOUND		= 52;//0x //called from NS_SUBCODE_RFRECV
    public static final byte NS_SUBCODE_RFROUTEFAILED		= 53;//0x //called from NS_SUBCODE_RFSEND
    public static final byte NS_SUBCODE_RFGETROUTECOUNT		= 54;//0x //called from NS_SUBCODE_RFSEND
    public static final byte NS_SUBCODE_RFGETROUTECOUNTRES	= 55;//0x //response to NS_SUBCODE_RFGETROUTECOUNT
    public static final byte NS_SUBCODE_RFGETROUTE			= 56;//0x //called from NS_SUBCODE_RFSEND
    public static final byte NS_SUBCODE_RFGETROUTERES		= 57;//0x //response to NS_SUBCODE_RFGETROUTE

    //32-255: Code/sub-code specific
    public static final byte NS_NOK_RECV 				= 32;
    public static final byte NS_NOK_SEND 				= 33;
    public static final byte NS_NOK_BCAST 				= 34;
    public static final byte NS_NOK_KEY_TOO_LONG 		= 35;

    // Define node roles.
    public static final byte ROLE_ROUTER_NODE = 0x00;
    public static final byte ROLE_EDGE_NODE = 0x01;
    public static final byte ROLE_CONTROLLER_NODE = 0x03;
    public static final byte ROLE_GATEWAY_NODE = 0x03;

    // Defines network capabilities.
    public static final byte NWKCAPS_ALWAYS_LISTENING 	= 0x01;//always reachable
    public static final byte NWKCAPS_PERIODIC_WAKEUP 	= 0x02;//sleeps but wakes up periodically
    public static final byte NWKCAPS_ALWAYS_SLEEPING 	= 0x03;//never wakes up, only sends msgs
    public static final byte NWKCAPS_ROUTER 			= 0x04;//routing node
    public static final byte NWKCAPS_EDGE 				= 0x08;//edge node

    //define delivery methods
    /** Defines direct delivery. */
    public static final byte DELIVERY_DIRECT = 0x01;
    /** Defines routed message delivery. */
    public static final byte DELIVERY_ROUTED = 0x02;
    /** Defines flood routing message delivery. */
    public static final byte DELIVERY_FLOOD = 0x04;
    /** Defines exhaustive delivery approach. */
    public static final byte DELIVERY_EXHAUSTIVE = DELIVERY_DIRECT | DELIVERY_ROUTED | DELIVERY_FLOOD;
    
    //Positive values define success
    //Negative values define errors
    //Zero value undefined?

    //OK subCode group
    /** Message sent/received correctly. */
    public static final byte OK = 1;
    /** Internal message received and can be ignored. */
    public static final byte OK_MESSAGE_INTERNAL = 2;
    /** Message irrelevant for this node has been received and can be ignored. */
    public static final byte OK_MESSAGE_IGNORED = 3;
    /** Warning, ACK message too long. */
    public static final byte OK_WARNING_ACK_TOO_LONG = 4;

    //Wrong parameters subCode group
    /** Unknown delivery invalid or incompatible with destination. */
    public static final byte ERROR_DELIVERY_METHOD_INVALID = -10;
    /** Payload to long for delivery. */
    public static final byte ERROR_PAYLOAD_TOO_LONG = -11;
    /** Retry count is invalid. */
    public static final byte ERROR_INVALID_RETRY_COUNT = -12;

    //Internal network errors subCode group
    /** Message ignored due to max hops reached. */
    public static final byte ERROR_MESSAGE_IGNORED_MAX_HOPS_REACHED = -20;

    //ACK errors subCode group
    /** No acknowledge received. */
    public static final byte ERROR_ACK_NOT_RECEIVED = -30;
    /** Sending an ACK has failed. */
    public static final byte ERROR_ACK_SEND_FAILED = -31;
    /** Flood message not received by neighbours. */
    public static final byte FLOOD_NOT_RECEIVED_BY_NEIGHBOURS = -32;

    //Routing errors subCode group
    /** No known routes to the destination. */
    public static final byte ERROR_NO_KNOWN_ROUTES = -40;
    /** Rerouting a message has failed. */
    public static final byte ERROR_REROUTE_FAILED = -41;

    /** First possible node ID. */
    public static final byte MIN_NODE_ID 	= 1;
    /** Last possible node ID. */
    public static final byte MAX_NODE_ID 	= (byte) 254;
    /** Maximum node count in the network. */
    public static final byte MAX_NODE_COUNT = MAX_NODE_ID - MIN_NODE_ID;

    /** Maximum length of a network key. */
    public static final byte MAX_NETWORK_KEY_LEN 	= 8;

}
