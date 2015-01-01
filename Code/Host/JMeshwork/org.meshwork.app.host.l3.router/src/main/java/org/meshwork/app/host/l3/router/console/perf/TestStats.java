package org.meshwork.app.host.l3.router.console.perf;

import java.io.PrintWriter;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public abstract class TestStats {

    public int testUID;
    public int runTime;
    public int runCount;
    public int successCount;
    public int failCount;
    public AbstractTestConfiguration config;

    public AbstractTestConfiguration getTestConfiguration() {
        return config;
    }

    public int getTestUID() {
        return testUID;
    }

    public int getRunTime() {
        return runTime;
    }

    public int getRunCount() {
        return runCount;
    }

    public int getSuccessCount() {
        return successCount;
    }

    public int getFailCount() {
        return failCount;
    }

    public abstract String getTestName();
    public abstract String getTestDescription();
    public abstract String getTestDetails();

    public void printTestStats(PrintWriter writer) {
        int runCount = getRunCount();
        int runTime = getRunTime();
        writer.println("Test Name: "+getTestName());
        writer.println("\t    Success count: "+getSuccessCount());
        writer.println("\t       Fail count: "+getFailCount());
        if ( runCount > 0 ) {
            writer.println("\t        Run count: "+runCount);
            writer.println("\t Success rate (%): "+100 * (runCount == 0 ? 0f : ((float)getSuccessCount() / runCount)));
        }
        if ( runTime > 0 ) {
            writer.println("\t     Run time (s): "+runTime);
            writer.println("\tSend rate (msg/s): "+(runTime == 0 ? 0f : ((float)getRunCount() / runTime)));
        }
        writer.println("\t      Description: "+getTestDescription());
        writer.println("\t          Details: "+getTestDetails());
        writer.println();
    }

}
