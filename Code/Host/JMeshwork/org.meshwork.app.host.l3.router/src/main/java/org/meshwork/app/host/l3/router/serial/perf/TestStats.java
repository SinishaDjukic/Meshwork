package org.meshwork.app.host.l3.router.serial.perf;

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

}
