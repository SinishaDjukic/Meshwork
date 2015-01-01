package org.meshwork.app.host.l3.router.console.perf;

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
            stats.printTestStats(writer);
        }
    }

}
