package org.meshwork.core.zeroconf.l3;

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
        messageTypes = new AbstractMessage[] {new MNOK((byte)0), new MOK((byte)0),
                                                new MUnknown((byte)0), new MZCCfgNwk((byte)0),
                                                new MZCCfgRep((byte)0), new MZCDeinit((byte)0),
                                                new MZCID((byte)0), new MZCIDRes((byte)0),
                                                new MZCInit((byte)0), new MZCNwkID((byte)0),
                                                new MZCNwkIDRes((byte)0)
        };
        hash = new HashMap<Byte, AbstractMessage>();
        for (AbstractMessage messageType : messageTypes) {
            hash.put(messageType.getCode(), messageType);
        }
    }

    @Override
    public AbstractMessage deserialize(MessageData data) throws IOException {
        //TODO the caller must ensure that:
        // 1) data.data != null unless data.len == 1
        // 2) data.len == data.data.length + 1
        AbstractMessage messageType = hash.get(data.code);
        AbstractMessage result = messageType != null ? messageType.deserialize(data) : null;
        return result;
    }

}
