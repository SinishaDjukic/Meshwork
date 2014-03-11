package org.meshwork.core.host.l3;

import java.util.Vector;

/**
 * Created by Sinisha Djukic on 14-2-21.
 */
public class RouteList {

    protected Vector<Route> list;

    public RouteList() {
        list = new Vector(8);
    }

    public int getRouteCount() {
        return list.size();
    }

    public Route getRoute(int index) {
        return list.elementAt(index);
    }

    public void addRoute(Route route) {
        list.add(route);
    }

    public void removeRoute(Route route) {
        list.remove(route);
    }

    public int indexOf(Route route) {
        return list.indexOf(route);
    }

    public Route getRoute(Route route, boolean autoCreate, byte autoCreateSrcID) {
        Route result = null;
        int index = list.indexOf(route);
        if ( index != -1 ) {
            result = list.elementAt(index);
        } else if ( autoCreate ) {
            result = new Route();
            result.src = autoCreateSrcID;
            list.add(result);
        }
        return result;
    }

}
