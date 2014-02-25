package org.meshwork.core;

/**
 * Created by Sinisha Djukic on 14-2-14.
 */
public class TransportTimeoutException extends Exception {

    public TransportTimeoutException() {
        super();
    }

    public TransportTimeoutException(String message) {
        super(message);
    }

    public TransportTimeoutException(String message, Throwable cause) {
        super(message, cause);
    }

    public TransportTimeoutException(Throwable cause) {
        super(cause);
    }

}
