# Building Tableau Log Viewer

TLV is build using C++ and Qt libraries and can be build in different platforms.

You can get the Qt libraries from here the [Qt Site](https://www.qt.io/download/). It is fine to use the Open Source version.

**Note:** When installing Qt make sure you get the following components: `Qt WebView` and `Qt WebEngine`.

## Building on Mac (using command line)
The following instructions use Qt 5.7 and the clang compiler to compile a 64-bit binary.
Building TLV with a different Qt version or compiler should be fairly similar.

```bash
# Start by cloning the repository
git clone https://github.com/tableau/tableau-log-viewer.git
cd tableau-log-viewer

# Find out where the Qt libraries are stored in your disk drive, and add the directory to the PATH
export PATH="~/Qt/5.7/clang_64/bin":$PATH

# Create a directory to compile
mkdir build-release && cd build-release

# Build
qmake -spec macx-clang CONFIG+=x86_64 -Wall ../src/tableau-log-viewer.pro
make

# Make a self-contained app
macdeployqt tlv.app

# Run the application
open tlv.app
```


## Building on Windows (using command line)
The following instructions use Qt 5.6 and the Visual Studio 2013 compiler (mscv2013) to compile a 64-bit binary.
Building TLV with a different Qt version or compiler should be fairly similar.

The following instructions use two command line windows:
* The **Git Bash** terminal that you get from a standard Git installation on Windows. Search for "Git Bash" in the start menu.
* The **Visual Studio Tools Command Prompt** that you get from a Visual Studio installation.
There's a link from the Start Menu -> Visual Studio 2013 -> Visual Studio Tools.
It is called "VS2013 x64 Native Tools Command Prompt".

**From Git Bash:**
```bash
# Start by cloning the repository
git clone https://github.com/tableau/tableau-log-viewer.git
cd tableau-log-viewer

# Find out where the Qt libraries are stored in your disk drive, and add the directory to the PATH
export PATH=$PATH:"/c/Qt/5.6/msvc2013_64/bin"

# Make build directory and run QMake
mkdir build-release
cd build-release
qmake -spec win32-msvc2013 CONFIG+=x86_64 -Wall ../src/tableau-log-viewer.pro
```
To compile, the enviroment has be set up to use the correct tools. The easiest way is to use the Visual Studio prompt.

**From the Visual Studio command prompt:**
```cmd
cd \path\to\tableau-log-viewer\build-release
nmake
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
Start by cloning the repository
```bash
git clone https://github.com/tableau/tableau-log-viewer.git
```

1. Launch Qt Creator and open the project file, it should be under *src/tableau-log-viewer.pro*
2. Configure the new project. The default configuration might be good enough, if you have problems see the section "Fixing configuration"
3. Click on the Build button (the one with the Hammer)
4. Click on the Run button

If everything went well, TLV should up and running!

### Fixing configuration
The project can be configured with different compiler and targets.

To change the configuration:

1. Click on the Projects button (the one with a wrench)
2. On the "Build & Run" section, click on "Manage kits..."

We have only tried a handful of configurations. Here are some that are known to work (strings as displayed by Qt creator)

#### Windows, Visual Studio 2013 compiler, 64 bit
* Compiler: Microsoft Visual C++ Compiler 12.0 (amd64)
* Debugger: CDB 64 bit
* Qt Version: Qt 5.X.X MSVC2013 64bit

#### Mac OS X, clang compiler, 64 bit
* Compiler: Clang (x86 64 bit in /usr/bin)
* Debugger: System LLDB at /usr/bin/lldb
* Qt Version: Qt 5.X.X clang 64bit
