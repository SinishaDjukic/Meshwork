package org.meshwork.app.host.l3.router.serial.perf;

import java.io.PrintWriter;
import java.util.ArrayList;

/**
 * Created by Sinisha Djukic on 14-2-25.
 */
public class TestSummaryStats {

    public ArrayList<TestStats> testStats;

    public TestSummaryStats(ArrayList<TestStats> testStats) {
        this.testStats = testStats;
    }

    public ArrayList<TestStats> getTestStats() {
        return testStats;
    }

    public void printResults(PrintWriter writer) {
        int tests = testStats.size();
        writer.println("Total Test Cases: "+tests);
        writer.println("-------------------");
        for ( int i = 0; i < tests; i ++ ) {
            TestStats stats = testStats.get(i);
            int runCount = stats.getRunCount();
            int runTime = stats.getRunTime();
            writer.println("Test Name: "+stats.getTestName());
            writer.println("\tSuccess count: "+stats.getSuccessCount());
            writer.println("\t   Fail count: "+stats.getFailCount());
            writer.println("\t    Run count: "+runCount);
            writer.println("\t Success rate: "+(runCount == 0 ? 0f : (stats.getSuccessCount() / runTime)));
            writer.println("\t Run time (s): "+stats.getRunTime());
            writer.println("\tSend rate (s): "+(runTime == 0 ? 0f : (stats.getRunCount() / runTime)));
            writer.println("\t  Description: "+stats.getTestDescription());
            writer.println("\t      Details: "+stats.getTestDetails());
            writer.println();
        }
    }

}