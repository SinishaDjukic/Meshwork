@echo off
echo ..............
java -version
echo ..............
cd /D %~dp0
set CMDLINE=java -Xmx256M -classpath lib/org.meshwork.core.api.jar;lib/org.meshwork.core.host.l3.jar;lib/org.meshwork.core.transport.serial.jssc.jar;lib/org.meshwork.app.host.l3.router.jar;lib/jssc.jar org.meshwork.app.host.l3.router.console.ConsoleRouterImpl %1 %2 %3 %4 %5

echo %CMDLINE%

%CMDLINE%

pause