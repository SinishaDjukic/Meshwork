package org.meshwork.core;

import java.io.IOException;

/**
 * Created by Sinisha Djukic on 14-2-11.
 */
public interface AbstractMessageTransport {

    public static final int SEND_OK = 0;
    public static final int SEND_NOK = -1;

    public MessageData readMessage(int timeout) throws TransportTimeoutException, IOException;

    public int sendMessage(MessageData message) throws IOException;
    
    public boolean isOpen();
}
