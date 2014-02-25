package org.meshwork.core.util;

import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-13.
 */
public interface Printable {
    public void toString(PrintWriter writer, String rowPrefix, String rowSuffix, String separator);
}
