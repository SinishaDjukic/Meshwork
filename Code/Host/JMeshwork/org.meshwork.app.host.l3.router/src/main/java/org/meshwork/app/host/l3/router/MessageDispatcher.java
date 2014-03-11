package org.meshwork.app.host.l3.router;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public interface MessageDispatcher extends Runnable {

    void init() throws Exception;

    void deinit() throws Exception;
}
