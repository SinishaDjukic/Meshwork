Ex_Beacon
=========
Periodically broadcasts a message containing:
 - Sequence number (1 byte)
 - Temperature (2 bytes, 12.4 fixed point)
 - Device voltage (2 bytes, mV)
Temperature is provided by DS18B20, but can be disabled by setting "-D EX_TEMP_DISABLE=true" in the build options.
Network ID, Channel ID, Node ID and destination Port are set compile-time.


Ex_Router
=========
Generic message router. Reroutes messages for other nodes automatically. Payload is printed to the serial, unless disabled compile-time.
Network ID, Channel and Node ID are set compile-time.


Ex_Router_Temperature
=====================
Message router with support for Ex_Beacon payload parsing, which is then printed to the serial, unless disabled compile-time.
Network ID, Channel ID, Node ID are set compile-time, as well as the Port, on which Ex_Beacon messages are expected.


Ex_Router_ZeroConf
==================
Generic message router. Reroutes messages for other nodes automatically. Payload is printed to the serial, unless disabled compile-time.
Network ID, Channel ID, Node ID are configured via the Meshwork ZeroConf serial protocol, e.g. via the L3ZeroConfGUI tool. Destination Port is set compile-time.


Ex_Beacon_ZeroConf
==================
Periodically broadcasts a message containing:
 - Sequence number (1 byte)
 - Temperature (2 bytes, 12.4 fixed point)
 - Device voltage (2 bytes, mV)
Temperature is provided by DS18B20, but can be disabled by setting "-D EX_TEMP_DISABLE=true" in the build options.
Network ID, Channel ID, Node ID are configured via the Meshwork ZeroConf serial protocol, e.g. via the L3ZeroConfGUI tool. Destination Port is set compile-time. Notes about the delivery options:
 - If Node ID is set to 255 then the message will be broadcasted, otherwise it will be sent via the configured delivery options
 - If Node ID is not set to 255 then delivery options have an effect and you can choose a combination of DIRECT, ROUTED and FLOOD


Ex_Console
==========
Interactive serial console exposing most of the low-level functionality.


Ex_Serial_Router
================
Generic message router. Reroutes messages for other nodes automatically. All messages targeted to this node will be forwared to the serial port via the Meshwork serial protocol. L3RouterConsole can be used on the host to process the messages in Java. The host can receive the payload, respond with a reply in the ACK message, send/broadcast messages originating from this node, handle the routing tables, etc.

