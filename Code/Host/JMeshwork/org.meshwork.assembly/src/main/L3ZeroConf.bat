@echo off
echo ..............
java -version
echo ..............
cd /D %~dp0
set CMDLINE=java -Xmx256M -classpath lib/org.meshwork.core.api.jar;lib/org.meshwork.core.host.l3.jar;lib/org.meshwork.core.transport.serial.jssc.jar;lib/org.meshwork.core.zeroconf.l3.jar;lib/org.meshwork.app.zeroconf.l3.node.jar;lib/jssc.jar org.meshwork.app.zeroconf.l3.node.console.ConsoleNodeImpl %1 %2 %3 %4 %5

echo %CMDLINE%

%CMDLINE%

pause