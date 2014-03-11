package org.meshwork.core.util;

import java.util.ArrayList;
import java.util.StringTokenizer;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public class Converter {
    
    public static byte toByte(String propKey, String propValue) {
        if ( propValue == null )
            throw new IllegalArgumentException("Property '"+propKey+"' cannot be null!");
        return Byte.parseByte(propValue);
    }

    public static ArrayList<Byte> toByteArrayList(String propKey, String propValue) {
        if ( propValue == null )
            throw new IllegalArgumentException("Property '"+propKey+"' cannot be null!");
        ArrayList<Byte> result = new ArrayList<Byte>(16);
        StringTokenizer st = new StringTokenizer(propValue, ", ", false);
        while (st.hasMoreElements()) {
            String dst = (String) st.nextElement();
            if ( dst != null ) {
                try {
                    result.add(toByte(dst, dst));
                } catch (Throwable t) {
                    System.err.println("Error converting '"+dst+"' to byte due to: "+t.getMessage());
                }
            }
        }
        return result;
    }

    public static short toShort(String propKey, String propValue) {
        if ( propValue == null )
            throw new IllegalArgumentException("Property '"+propKey+"' cannot be null!");
        return Short.parseShort(propValue);
    }

    public static boolean toBoolean(String propKey, String propValue) {
        if ( propValue == null )
            throw new IllegalArgumentException("Property '"+propKey+"' cannot be null!");
        return Boolean.parseBoolean(propValue);
    }

    public static int toInt(String propKey, String propValue) {
        if ( propValue == null )
            throw new IllegalArgumentException("Property '"+propKey+"' cannot be null!");
        return Integer.parseInt(propValue);
    }

}
