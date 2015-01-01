package org.meshwork.app.host.l3.router.console.perf;

import java.io.InputStream;

/**
 * Created by Sinisha Djukic on 14-2-26.
 */
public abstract class AbstractTestConfiguration {
    //delay between iterations
    public int iterationDelay;
    //iterations by count
    public int maxIterations;
    //iterations by time in seconds
    public int maxTime;

    public abstract void loadConfiguration(InputStream is) throws Exception;

    public int getIterationDelay() {
        return iterationDelay;
    }

    public int getMaxTime() {
        return maxTime;
    }

    public int getMaxIterations() {
        return maxIterations;
    }
}
