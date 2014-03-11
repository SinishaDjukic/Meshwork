package org.meshwork.core.host.l3;

import java.util.HashMap;

/**
 * Created by Sinisha Djukic on 14-2-21.
 */
public class RouteMap {

    protected final HashMap<Byte,RouteList> map;

    public RouteMap(){
        map = new HashMap<Byte, RouteList>();
    }

    public RouteList getRouteList(byte dst, boolean autoCreate) {
        RouteList result = map.get(dst);
        if ( result == null && autoCreate ) {
            result = new RouteList();
            map.put(dst, result);
        }
        return result;
    }

    public void addRouteList(byte dst, RouteList list) {
        RouteList result = map.remove(dst);
        map.put(dst, list);
    }

    public void clearRouteList() {
        map.clear();
    }
}
