There are six projects in this folder. These projects can only be executed in a Linux environment.

You must install premake4:
`sudo apt-get install premake4`

Verify your premake4 version (version 4.3 has been tested to work):
`premake4 --version`

You must also install cmake:
https://cmake.org/download

Verify your cmake version (version 3.10.2 has been tested to work):
`cmake --version`

Then, from this folder, run:
`premake4 gmake`
`make`

Then, `cd` into any of the following folders:
- A0: triangle
- A1: cube
- A2: puppet
- A3: 
- A4
- Pool

Read the README in that subfolder for further instructions.

Note: you may need to install various development packages first if the premake4 commands fail. Try:
`sudo apt-get install mesa-common-dev mesa-utils-extra libgl1-mesa-dev libglapi-mesa doxygen xorg-dev libglu1-mesa-dev`

