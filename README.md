# VISE - Visual Inspection Software for Electronics

## Description
The Visual Inspection Software for Electronics (VISE) is used for inspecting pictures/scans of printed circuited board assemblies. Its main goal is to allow quick and easy verification of all electronic components to assess their state (fitted, not fitted, misaligned, bad solder, broken, etc.). With the help of this tool, an inspector can automatically find any component on the board without resorting to a slow and error-prone visual search.

For more information about the software, refer to the User Manual found in the "User Manual" folder of this project.

## View of the software
![Software during inspection mode](User%20Manual/Images/Inspection.png)

## Installing dependencies
These instructions assume building on Windows, starting from scratch, but the process should be very similar for other OSs. The compiler used is MinGW (GNU C++ compiler for Windows) but this should all be feasible with a different compiler.

### Compiler: MinGW
- Install [MSYS2](https://www.msys2.org/). This software will easily setup your machine for compilation using GCC
- It is recommended to install this folder at the root of your machine (C:\msys64)
- In the msys64 folder, run mingw64.exe
- Type the following commands:
  - pacman -Syuu  (Updates "pacman" the software/package manager)
  - Type the previous command until there are no more updates
  - pacman -S mingw-w64-x86_64-toolchain (When asked, choose gcc (option 3))
  - pacman -S mingw-w64-i686-toolchain (Facultative, for the 32 bits tools) (When asked, choose gcc (option 3))
  - pacman -S mingw-w64-x86_64-gdb (Debugger)
  - pacman -S mingw-w64-i686-gdb (Facultative, for the 32 bits tools) (Debugger)
  - pacman -S mingw-w64-x86_64-cmake (To configure compilation of some software with CMake)
  - pacman -S mingw-w64-x86_64-make (To compile some software with Make)
  - Open a Windows admin console
	  - Browse to the following folder "C:\msys64\mingw64\bin" then type the command:
	  - mklink make mingw32-make.exe

All done!

### Librairie: nativefiledialog-extended 
This is a librairie that allows easy and portable access to the open/save file dialogs of the OS.
- Download the .zip source code from https://github.com/btzy/nativefiledialog-extended
- Follow the instructions of the "Standalone Library" section of the Github to compile the .lib
	- Open a mingw64 console (from MSYS2)
	- Browse to the unzipped archive (when browsing, the root is /c/Users/...)
	- Create a "build" folder and enter it
		- mkdir build
		- cd build
	- Type the following command to configure with CMAKE:
		- "cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release .."
	- Type the following command to build with gcc
		- "cmake --build ."
	- ... fingers crossed
- Copy the file "build/src/libnfd.a" to a new "lib" folder at the root (with build, docs, screens, etc.)
- Copy the "src/include" folder at the root too
- Move the "nativefiledialog-extended" folder to where you store your librairies. Example: "C:\MyLib"

### Librairie: SDL2
This is a portable librairie to manage windows, events and graphical interface as a whole.
- Download the precompiled version from Github: https://github.com/libsdl-org/SDL/releases
	- It should be SDL2-devel-2.26.3-mingw.zip or more recent
- Unzip
- Create the folders "bin", "include" and "lib" at the root
- Copy respectively the content of "x86_64-w64-mingw32\include\SDL2", "x86_64-w64-mingw32\lib" et "x86_64-w64-mingw32\bin" into the previously created folders
- Move the "SDL2-2.26-3" folder to where you store your librairies

### Librairie: SDL2_image
This is an extension to the SDL2 library. It allows opening and saving image files (PNG, JPG, GIF, etc.)
- Download the precompiled version from Github: https://github.com/libsdl-org/SDL_image/releases
	- It should be SDL2_image-devel-2.6.3-mingw.zip or more recent
- Repeat the same steps as for SDL2

### Librairie: SDL2_ttf
This is an extension to the SDL2 library. It allows generation of text images based on fonts (ttf and otf)
- Download the precompiled version from Github: https://github.com/libsdl-org/SDL_ttf/releases
	- It should be SDL2_ttf-devel-2.20.2-mingw.zip or more recent
- Repeat the same steps as for SDL2

### IDE: Code::Blocks
This software is used to open the project files and help with the configuration of dependencies.
- Install [Code::Blocks](http://www.codeblocks.org/)
- If during the installation you are requested to fill the compiler information, do so. If not, follow to instruction below after installation
- Open the menu "Settings->Compiler..."
- For "Selected compiler" choose GNU GCC Compiler
- Open the tab "Toolchain executables"
- Fill the information
	- Compiler's installation directory should be something like: C:\msys64\mingw64
	- C Compiler: gcc.exe
	- C++ compiler: g++.exe
	- Linker for dynamic libs: g++.exe
	- Linker for static libs: ar.exe
	- Debugger: GDB/CDB debugger: Default
	- Resource compiler: windres.exe
	- Make program: mingw32-make.exe

## Building
Now that all tools have been configured, the project can be opened and the software can be built.
- Open the project file: [VisualInspection.cbp](Source/VisualInspection.cbp)
- Opening the menu Project->Build Options
- From the list on the left, choose the project name "VisualInspection"
- Open the "Search directories" tab
- In the sub-tab "Compiler" notice that all paths are global variables
	- Example: "$(#sdl2.include)" points to the field "include" of the global variable "sdl2" 
- In the sub-tab "Linker" they should all be variables too
- Having global variables makes the project file identical for everyone regardless of the path where they store their librairies.
- You must set these variables in your Code::Blocks software:
	- Open the menu "Settings->Global Variables"
	- Press the "New" button under "Current variable"
	- Fill the fields base, include, lib and bin. Example for SDL2
		- base: C:\MyLib\64bits\SDL2-2.26.3
		- include: C:\MyLib\64bits\SDL2-2.26.3\include
		- lib: C:\MyLib\64bits\SDL2-2.26.3\lib
		- bin: C:\MyLib\64bits\SDL2-2.26.3\bin
	- All 3 other librairies should be similar, except NativeFileDialog which does not have/need a "bin" folder
- When built, the software's objects (.obj) and executables (.exe) will be placed in a specific folder. In my case, it is near the root in a folder where the antimalware software won't flag it. This will need to be adjusted if you prefer another path. To do so:
	- Open the menu "Projet->Properties..."
	- Choose the tab "Build targets"
	- For the "Build targets" Debug and Release, if needed, change the "Output filename"
	- If needed change Objects output dir:
- After setting up your environment, you can now build the software
- The first time you build it, you should use the button "Rebuild" (two blues arrows)
- For all subsequent builds you can use the button "Build" (gear) or "Build and run" (gear and green arrow)

If you want to run the executable by double-clicking the .exe (instead of hitting the "Run" button) you will need to place/copy the res_images and res_fonts folder in the same folder and put the following .dll with it. The dll are found in the bin folders of the dependencies(C:\MyLib\64bits\SDL2-2.26.3\bin) and the compiler (C:\msys64\mingw64\bin)
- libgcc_s_seh-1.dll (Exception handling (SEH))
- libstdc++-6.dll (C++ Standard Library)
- libwinpthread-1.dll (PThreads implementation on Windows (Threading))
- SDL2.dll
- SDL2_image.dll
- SDL2_ttf.dll