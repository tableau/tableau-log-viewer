# Using a pre-built version of Tableau Log Viewer

We provide [pre-built releases](https://github.com/tableau/tableau-log-viewer/releases) of Tableau Log Viewer.
No need to setup a dev-environment if all you really want to do is inspecting your lovely log files.

Download the ZIP file of the [latest release](https://github.com/tableau/tableau-log-viewer/releases/latest) for your operating system (Windows and Mac OS available) and extract the ZIP file. Finally, launch the application (tlv.exe for Windows and tlv.app for Mac).

# Building Tableau Log Viewer

TLV is built using C++ and Qt libraries and can be build in different platforms.

The easiest way to install the Qt libraries is through [aqtinstall](https://github.com/miurahr/aqtinstall):

```
pip install aqtinstall
# On OSX
aqt install-qt mac desktop 6.4.0 clang_64 -m qtwebview qtwebengine qtpositioning qtwebchannel
# On Windows
python -m aqt install-qt windows desktop 6.4.0 win64_msvc2019_64
```

**Note:** When installing Qt make sure you get the following components: `Qt WebView` and `Qt WebEngine`.

## Building on Mac (using command line)
The following instructions use Qt 6.4 and the clang compiler to compile a 64-bit binary.
Building TLV with a different Qt version or compiler should be fairly similar.

```bash
# Start by cloning the repository
git clone https://github.com/tableau/tableau-log-viewer.git
cd tableau-log-viewer

# Find out where the Qt libraries are stored in your disk drive, and add the directory to the PATH
export PATH="~/Qt/6.4.0/macos/bin":$PATH

# Create a directory to compile
mkdir build-release && cd build-release

# Build
qmake -spec macx-clang -Wall ../src/tableau-log-viewer.pro
make

# Make a self-contained app
macdeployqt tlv.app

# Run the application
open tlv.app
```

## Building on Linux

Steps for Ubuntu 22.04

### Install Qt 6.2 components

```bash
sudo apt install qt6-webengine-dev qt6-base-dev qmake6
```

### Install OpenGL components

```bash
sudo apt-get install -y freeglut3-dev
```

### Build

```bash
mkdir -p build-release
cd build-release
qmake6 -spec linux-g++-64 QMAKE_CC=gcc QMAKE_CXX=g++ -Wall ../src/tableau-log-viewer.pro
make -j 8
tlv
```

## Building on Windows (using command line)

The following instructions use Qt 6.4 and the Visual Studio 2022 compiler to compile a 64-bit binary.
Building TLV with a different Qt version or compiler should be fairly similar.

You can install the MSVC compiler by downloading "Build Tools for Visual Studio 2022" from [https://visualstudio.microsoft.com/downloads/#other] 
and selecting "Desktop development with C++" from the installer.
A full-blown Visual Studio installation is not necessary.

The following instructions use the **Visual Studio Command Prompt** that you get from a Visual Studio installation.
You can find it from the Windows menu, it is called "x64 Native Tools Command Prompt for VS 2022".

**From the Visual Studio command prompt:**
```cmd
# Start by cloning the repository
cd where\ever\you\store\your\code
git clone https://github.com/tableau/tableau-log-viewer.git


# Make build directory and run QMake
mkdir build-release
cd build-release
C:\Qt\6.4.0\msvc2019_64\bin\qmake -spec win32-msvc -Wall ../src/tableau-log-viewer.pro

# Build it
nmake

# Run it
.\release\tlv.exe
```

The binary will appear under build-release/release.
The executable needs the Qt libraries to run. The Git Bash command prompt already had the Qt libraries on the PATH.

**Back to the Git Bash:**
```bash
# Run the executable
cd release
./tlv.exe
```

## Buiding using Qt Creator

** This section is outdated. If you successfully built on Linux, please update this section. **

Start by cloning the repository
```bash
git clone https://github.com/tableau/tableau-log-viewer.git
```

1. Launch Qt Creator and open the project file, it should be under *src/tableau-log-viewer.pro*
2. Configure the new project. The default configuration might be good enough, if you have problems see the section "Fixing configuration"
3. Click on the Build button (the one with the Hammer)
4. Click on the Run button

If everything went well, TLV should be up and running!

### Fixing configuration
The project can be configured with different compiler and targets.

To change the configuration:

1. Click on the Projects button (the one with a wrench)
2. On the "Build & Run" section, click on "Manage kits..."

We have only tried a handful of configurations. Here are some that are known to work (strings as displayed by Qt creator)

#### Windows, Visual Studio 2013 compiler, 64 bit
* Compiler: Microsoft Visual C++ Compiler 12.0 (amd64)
* Qt Version: Qt 5.X.X MSVC2013 64bit
* [Optional but recommended] Add a 64 bit debugger
  1. Switch to the "Debuggers" tab.
  2. Click on the "Add" button
  3. Give a name you can recognize later like "CDB 64 bit"
  4. Find the 64 bit binary. It should be under `C:\debuggers\x64\cdb.exe` or similar
  5. Save the changes clicking the "Apply" button
  6. Go back to the "Kits" tab and choose the debugger configuration just created

#### Mac OS X, clang compiler, 64 bit
* Compiler: Clang (x86 64 bit in /usr/bin)
* Debugger: System LLDB at /usr/bin/lldb
* Qt Version: Qt 5.X.X clang 64bit
