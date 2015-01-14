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
        messageTypes = new AbstractMessage[] {new MNOK((byte)0), new MOK((byte)0),
                                                new MUnknown((byte)0), new MZCDeinit((byte)0),
                                                new MZCDevCfg((byte)0), new MZCDevReq((byte)0),
                                                new MZCDevRes((byte)0), new MZCInit((byte)0),
                                                new MZCNwkCfg((byte)0) ,new MZCNwkReq((byte)0),
                                                new MZCNwkRes((byte)0), new MZCRepCfg((byte)0),
                                                new MZCRepReq((byte)0), new MZCRepRes((byte)0),
                                                new MZCSerialCfg((byte)0), new MZCSerialReq((byte)0),
                                                new MZCSerialRes((byte)0)
        };
        hash = new HashMap<Byte, AbstractMessage>();
        for (AbstractMessage messageType : messageTypes) {
            hash.put(messageType.getSubCode(), messageType);
        }
    }

    @Override
    public AbstractMessage deserialize(MessageData data) throws IOException {
//        System.out.println("[DESERIALIZE] data.subCode="+data.subCode+", data="+data);
        AbstractMessage messageType = hash.get(data.subCode);
//        System.out.println("[DESERIALIZE] type subcode="+messageType.getSubCode()+"type found="+messageType);
        AbstractMessage result = messageType != null ? messageType.deserialize(data) : null;
        return result;
    }

}
