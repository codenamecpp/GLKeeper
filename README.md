# Grim Lands Keeper [GLKeeper]

![Alt text](/screenshots/logo.png?raw=true "Logo")


Open Source Dungeon Keeper 2 Reimplementation

### Status
It is barely started

### Tools and technologies used
* C++11
* OpenGL 3.2
* Box2D
* cJSON
* Dear ImGui
* STBI
* GLM math library
* GLFW
* GLEW

### Compiling on Linux

Before build project make sure to install dependencies:
* apt install libglew-dev libglm-dev libgl1-mesa-dev xorg-dev clang

Run these commands in terminal:
* git clone --recurse-submodules https://github.com/codenamecpp/GLKeeper
* cd GLKeeper
* make

### How to Run

Original Dungeon Keeper 2 game resources required in order to play (GOG or Retail version). It still can be purchased here https://www.gog.com/game/dungeon_keeper_2

Note that GLKeeper must be compiled and configured (optionally) before this step as described above.
To run it on Linux just type in console:
* cd GLKeeper
* run ARGS="-gamedir XXX -mapname YYY"

Here XXX is path to original game resources directory and YYY is map name to load.

### Screenshots

Windows 7 x64:
![Alt text](/screenshots/0_0_1.png?raw=true "Screenshot")
![Alt text](/screenshots/0_0_2.png?raw=true "Screenshot")

Linux Lite:
![Alt text](/screenshots/0_0_3_Linux.png?raw=true "Screenshot")
