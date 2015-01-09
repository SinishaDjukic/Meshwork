package org.meshwork.core.host.l3;

import org.meshwork.core.AbstractMessage;
import org.meshwork.core.MessageData;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-10.
 */
public class MRFRouteFound extends AbstractMessage implements Constants {

    public Route route;

    public MRFRouteFound(byte seq) {
        super(seq, NS_CODE, NS_SUBCODE_RFROUTEFOUND);
        route = new Route();
    }

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("MRFRouteFound: ");route.toString(writer, rowPrefix, rowSuffix, separator);
    }

    @Override
    public AbstractMessage deserialize(MessageData msg) throws IOException {
        MRFRouteFound result = new MRFRouteFound(msg.seq);
        result.route.hopCount = msg.data[0];
        result.route.src = msg.data[1];
        result.route.hops = new byte[result.route.hopCount];
        System.arraycopy(msg.data, 2, result.route.hops, 0, result.route.hopCount);
        result.route.dst = msg.data[2+route.hopCount];
        return result;
    }

    @Override
    public void serializeImpl(MessageData msg) {
        msg.data = new byte[2 + route.hopCount + 1];
        msg.data[0] = route.hopCount;
        msg.data[1] = route.src;
        if ( route.hopCount > 0 )
            System.arraycopy(route.hops, 0, msg.data, 2, route.hopCount);
        msg.data[2+route.hopCount] = route.dst;
    }
}
