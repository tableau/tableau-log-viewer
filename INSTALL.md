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
export PATH=$PATH:"~/Qt/5.7/clang_64/bin"

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
Instructions coming soon...
