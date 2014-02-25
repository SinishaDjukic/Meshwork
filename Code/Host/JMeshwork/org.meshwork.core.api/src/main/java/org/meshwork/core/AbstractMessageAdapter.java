package org.meshwork.core;

import java.io.IOException;

/**
 * Created by Sinisha Djukic on 14-2-11.
 */
public interface AbstractMessageAdapter {

    //create a new instance with filled data based on the message
    public abstract AbstractMessage deserialize(MessageData data) throws IOException;

}
