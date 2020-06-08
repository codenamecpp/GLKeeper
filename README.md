# Grim Lands Keeper [GLKeeper]

![Alt text](/screenshots/logo.png?raw=true "Logo")

Open Source Dungeon Keeper 2 Reimplementation

### Current development progress
Loading some maps, building and selling rooms, terrain tagging, top-down camera control... 
Not playable whatsoever.

### Features
* Windows and Linux support
* Custom engine C++11 / OpenGL 3.2

### Tools and technologies used
* [Box2D](https://github.com/erincatto/box2d)
* [cJSON](https://github.com/DaveGamble/cJSON) (with some modifications)
* [Dear ImGui](https://github.com/ocornut/imgui)
* [STBI](https://github.com/nothings/stb)
* [GLM](https://github.com/g-truc/glm)
* [GLFW](https://github.com/glfw/glfw)
* [GLEW](https://github.com/nigels-com/glew)

### Compiling on Linux

Before build project make sure to install dependencies:
* apt install libglew-dev libglm-dev libgl1-mesa-dev xorg-dev clang

Run these commands in terminal:
* git clone --recurse-submodules https://github.com/codenamecpp/GLKeeper
* cd GLKeeper
* make

### Compiling on Windows

You will need Visual Studio 2015/2017 Community Edition to build project. Before compile, add system environment variable SDKDIR with path where all the dependencies lives. These dependencies are:
* GLM

### How to Run

Original Dungeon Keeper 2 game resources required in order to play (GOG or Retail version). It still can be purchased here https://www.gog.com/game/dungeon_keeper_2

Note that GLKeeper must be compiled and configured (optionally) before this step as described above.
To run it on Linux just type in console:
* cd GLKeeper
* make run ARGS="-gamedir XXX -mapname YYY"

Here XXX is path to original game resources directory and YYY is map name to load.

### Screenshots

Windows 7 x64:
![Alt text](/screenshots/0_0_1.png?raw=true "Screenshot")
![Alt text](/screenshots/0_0_2.png?raw=true "Screenshot")

Linux Lite:
![Alt text](/screenshots/0_0_3_Linux.png?raw=true "Screenshot")
