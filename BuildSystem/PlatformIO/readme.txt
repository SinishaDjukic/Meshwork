PlatformIO Framework for Cosa
=============================
This PlaformIO framework for Cosa is developed and maintained by Sinisha Djukic as part of the Meshwork project.

NOTE: This framework provides UNOFFICIAL support for Cosa and is not in any way associated with the Cosa project or its author Mikael Patel. In case of questions, ideas, feature and change requests please visit:
https://github.com/SinishaDjukic/Meshwork

Configuration
=============
NOTE: Currently, only bare mimum instructions are provided and should be extended.

I. Installing the PlatformIO Framework for Cosa
In order to start building Meshwork projects you need to isntall the Cosa Framework into PlatformIO. Since at present this is an UNOFFICIAL effort to add Cosa support it cannot be installed automatically from the PlatformIO repositories and some manual work needs to be done on your development system. Instructions below assume you have already pulled the Cosa project from:
    https://github.com/mikaelpatel/Cosa/

Some symlinks need to be created in the PlatformIO's home folder residing "<home dir>./platformio" as follows:
    ./platformio/platforms/cosa -> <Meshwork project root>/Build/PlatformIO/platform/cosa
    ./platformio/packages/framework-cosa -> <Cosa project root>

II. Project Configuration
In order to compile Meshwork projects their "lib" folder is expected to have an extra folder, which is to be created for your system:
    <project root>/lib/Meshwork -> <Meshwork project root>\Code\Library\Meshwork>

Further, make sure you have the following values for your target board (env) in the project's platformio.ini file:
    platform = cosa
    framework = cosa


NOTE: The build system has so far been tested only on Windows, so some glitches may be expected on Linux, althouth it is supposed to be cross-platform.