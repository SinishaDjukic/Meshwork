package org.meshwork.core.host.l3;

import org.meshwork.core.util.Printable;
import org.meshwork.core.util.Printer;

import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-21.
 */
public class Route implements Printable {

    public byte hopCount;
    public byte src;
    public byte[] hops;
    public byte dst;

    //TODO add failed counters, reset methods, etc.
    public int statsFailed;
    public int statsFound;

    @Override
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator) {
        writer.print("Route[HopCount=");writer.print(hopCount);
        writer.print("Src=");writer.print(src);
        writer.print("Hops=");Printer.printHex(writer, hops, 0, 0, rowPrefix, rowSuffix, separator);
        writer.print("Dst=");writer.print(dst);
        writer.print("]");
    }

    public void addStatsFailed() {
        statsFailed++;
    }

    public void resetStatsFailed() {
        statsFailed = 0;
    }

    public int getStatsFailed() {
        return statsFailed;
    }

    public void addStatsFound() {
        statsFound++;
    }

    public void resetStatsFound() {
        statsFound = 0;
    }

    public int getStatsFound() {
        return statsFound;
    }

    public boolean equals(Object obj) {
        if ( obj == this )
            return true;
        Route r = (Route) obj;
        if ( hopCount != r.hopCount || src != r.src || dst != r.dst )
            return false;
        if ( hops != null && r.hops != null && hopCount > 0 ) {
            for ( int i = 0; i < hopCount; i ++ )
                if ( hops[i] != r.hops[i] )
                    return false;
        }
        return true;
    }

    //TODO pull serialization subCode from all related messages to here
}
