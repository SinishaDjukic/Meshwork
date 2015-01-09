package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.AbstractMessageAdapter;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.util.HashMap;

/**
 * Created by Sinisha Djukic on 14-2-11.
 */
public class MessageAdapter implements AbstractMessageAdapter {

    AbstractMessage[] messageTypes;
    HashMap<Byte, AbstractMessage> hash;

    public MessageAdapter() {
        //TODO use reflection to enumerate and instantiate
        messageTypes = new AbstractMessage[] {new MConfigBasic((byte)0), new MConfigNetwork((byte)0),
                                                new MRFBroadcast((byte)0), new MRFReceive((byte)0),
                                                new MRFReceiveACK((byte)0), new MRFSend((byte)0),
                                                new MRFStartReceive((byte)0), new MNOK((byte)0),
                                                new MOK((byte)0), new MUnknown((byte)0),
                                                new MInternal((byte)0), new MRFSendACK((byte)0),
                                                new MRFInit((byte)0), new MRFDeinit((byte)0),
                                                new MConfigRequest((byte)0), new MRFRouteFound((byte)0),
                                                new MRFRouteFailed((byte)0), new MRFGetRouteCount((byte)0),
                                                new MRFGetRouteCountRes((byte)0), new MRFGetRoute((byte)0),
                                                new MRFGetRouteRes((byte)0)
        };
        hash = new HashMap<Byte, AbstractMessage>();
        for (AbstractMessage messageType : messageTypes) {
            hash.put(messageType.getSubCode(), messageType);
        }
    }

    @Override
    public AbstractMessage deserialize(MessageData data) throws IOException {
        //TODO the caller must ensure that:
        // 1) data.data != null unless data.len == 1
        // 2) data.len == data.data.length + 1
        AbstractMessage messageType = hash.get(data.subCode);
        AbstractMessage result = messageType != null ? messageType.deserialize(data) : null;
        return result;
    }

}
