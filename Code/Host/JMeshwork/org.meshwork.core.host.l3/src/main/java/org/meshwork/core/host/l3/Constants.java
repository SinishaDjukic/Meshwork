package org.meshwork.core.host.l3;

/**
 * Created by Sinisha Djukic on 14-2-11.
 */
public interface Constants {

    public static final byte MAX_SERIALMSG_LEN 			= 64;//TODO calculate the right size!

    public static final byte MSGCODE_OK 				= 0;
    public static final byte MSGCODE_NOK 				= 1;
    public static final byte MSGCODE_UNKNOWN 			= 2;
    public static final byte MSGCODE_INTERNAL 			= 3;

    public static final byte MSGCODE_CFGBASIC 			= 10;
    public static final byte MSGCODE_CFGNWK 			= 11;
    public static final byte MSGCODE_RFINIT 			= 20;
    public static final byte MSGCODE_RFDEINIT 			= 21;
    public static final byte MSGCODE_RFRECV 			= 22;
    public static final byte MSGCODE_RFRECVACK 			= 23;
    public static final byte MSGCODE_RFSTARTRECV 		= 24;
    public static final byte MSGCODE_RFSEND 			= 25;
    public static final byte MSGCODE_RFSENDACK 			= 26;
    public static final byte MSGCODE_RFBCAST 			= 27;
    public static final byte MSGCODE_CFGREQUEST			= 28;
    public static final byte MSGCODE_RFROUTEFOUND		= 29;//called from MSGCODE_RFRECV
    public static final byte MSGCODE_RFROUTEFAILED		= 30;//called from MSGCODE_RFSEND
    public static final byte MSGCODE_RFGETROUTECOUNT	= 31;//called from MSGCODE_RFSEND
    public static final byte MSGCODE_RFGETROUTECOUNTRES	= 32;// response to MSGCODE_RFGETROUTECOUNT
    public static final byte MSGCODE_RFGETROUTE			= 33;//called from MSGCODE_RFSEND
    public static final byte MSGCODE_RFGETROUTERES		= 34;//response to MSGCODE_RFGETROUTE

    //0-63
    public static final byte ERROR_GENERAL 				= 0;
    public static final byte ERROR_INSUFFICIENT_DATA 	= 1;
    public static final byte ERROR_TOO_LONG_DATA 		= 2;
    public static final byte ERROR_ILLEGAL_STATE 		= 3;
    public static final byte ERROR_RECV 				= 4;
    public static final byte ERROR_SEND 				= 5;
    public static final byte ERROR_BCAST 				= 6;
    public static final byte ERROR_KEY_TOO_LONG 		= 7;
    public static final byte ERROR_SEQUENCE_MISMATCH	= 8;

    public static final short TIMEOUT_RESPONSE 			= 3000;

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

    //OK code group
    /** Message sent/received correctly. */
    public static final byte OK = 1;
    /** Internal message received and can be ignored. */
    public static final byte OK_MESSAGE_INTERNAL = 2;
    /** Message irrelevant for this node has been received and can be ignored. */
    public static final byte OK_MESSAGE_IGNORED = 3;
    /** Warning, ACK message too long. */
    public static final byte OK_WARNING_ACK_TOO_LONG = 4;

    //Wrong parameters code group
    /** Unknown delivery invalid or incompatible with destination. */
    public static final byte ERROR_DELIVERY_METHOD_INVALID = -10;
    /** Payload to long for delivery. */
    public static final byte ERROR_PAYLOAD_TOO_LONG = -11;
    /** Retry count is invalid. */
    public static final byte ERROR_INVALID_RETRY_COUNT = -12;

    //Internal network errors code group
    /** Message ignored due to max hops reached. */
    public static final byte ERROR_MESSAGE_IGNORED_MAX_HOPS_REACHED = -20;

    //ACK errors code group
    /** No acknowledge received. */
    public static final byte ERROR_ACK_NOT_RECEIVED = -30;
    /** Sending an ACK has failed. */
    public static final byte ERROR_ACK_SEND_FAILED = -31;

    //Routing errors code group
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
