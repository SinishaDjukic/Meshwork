@echo off
echo ..............
java -version
echo ..............
java -classpath lib/org.meshwork.core.api.jar;lib/org.meshwork.core.host.l3.jar;lib/org.meshwork.core.transport.serial.jssc.jar;lib/org.meshwork.app.host.l3.router.jar;lib/jssc.jar org.meshwork.app.host.l3.router.serial.Console %1 %2 %3 %4 %5