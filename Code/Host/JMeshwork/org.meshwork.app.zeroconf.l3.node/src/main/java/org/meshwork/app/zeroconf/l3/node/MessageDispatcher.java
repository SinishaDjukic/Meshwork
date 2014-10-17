package org.meshwork.app.zeroconf.l3.node;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public interface MessageDispatcher extends Runnable {

    void init() throws Exception;

    void deinit() throws Exception;
}
