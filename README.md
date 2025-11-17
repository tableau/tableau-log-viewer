# Tableau Log Viewer
[![Community Supported](https://img.shields.io/badge/Support%20Level-Community%20Supported-457387.svg)](https://www.tableau.com/support-levels-it-and-developer-tools)

**Master branch** | [![Travis-CI](https://img.shields.io/travis/tableau/tableau-log-viewer/master.svg?label=Linux%20build)](https://travis-ci.org/tableau/tableau-log-viewer) | [![AppVeyor](https://img.shields.io/appveyor/ci/tableau/tableau-log-viewer/master.svg?label=Windows%20build)](https://ci.appveyor.com/project/tableau/tableau-log-viewer/branch/master)
:--|---|---
**Dev branch** | [![Travis-CI](https://img.shields.io/travis/tableau/tableau-log-viewer/dev.svg?label=Linux%20build)](https://travis-ci.org/tableau/tableau-log-viewer) | [![AppVeyor](https://img.shields.io/appveyor/ci/tableau/tableau-log-viewer/dev.svg?label=Windows%20build)](https://ci.appveyor.com/project/tableau/tableau-log-viewer/branch/dev)

Tableau Log Viewer is a cross-platform tool with a simple interface that has a single purpose of making it easy to quickly glance over Tableau log files.

![TLV Screenshot](https://user-images.githubusercontent.com/1087437/45051694-a6a11880-b039-11e8-8028-969eb68e7c2b.png "TLV running on Windows 10")

Overview
---------------
* Tableau log file representation in easy to read columnar format
* Live capture from Tableau's log files
* Advanced event search, filtering and highlighting
* Query syntax highlighting
* Compatible with Windows, Mac and, most likely, Linux

How do I use Tableau Log Viewer?
---------------
You can get the latest release in the [Releases Section](https://github.com/tableau/tableau-log-viewer/releases). Download the ZIP file of the latest release for your operating system (Windows and Mac OS available) and extract the ZIP file. Finally, launch the application (tlv.exe for Windows and tlv.app for Mac).

To view logs, drag'n'drop a Tableau log file into the application. Both Tableau Desktop and [most] Tableau Server log files are supported.
Take a look at our [wiki](https://github.com/tableau/tableau-log-viewer/wiki) to get more details on the features and usage of TLV.

### Using Pre-Built Tableau Log Viewer on newer Mac machines

If you are trying to open a downloaded pre-built version of Tableau Log Viewer on an Apple Silicon Mac, this may fail as the application is not code-signed.
You can code-sign the application yourself to work around this limitation.

1. Create a self signed cert specifically for code signing
    1. Open the "Keychain access" utility. You might get prompted to "Open Passwords" or "Open Keychain Access", select the latter.
    2. Go to Menu > Keychain access > Certificate Assistant > Create Certificate...
    3. Provide a name like "self-signed-cert-for-coding", select Identity: "Self Signed Root", select Certificate type: "Code Signing"
    4. Create certificate
2. Sign the TLV application. In a terminal do:
    ```
    # Go to the directory where you copied the tlv application, assuming it was the Application folder, you would do:
    > cd /Applications
    # Use the name of your certificate
    > sudo codesign -fs "self-signed-cert-for-coding" --deep tlv.app
    ```
3. Try to open TLV. If it still being blocked, go to next step
4. Open System Settings > Privacy And Security. Scroll down a look for a notice about tlv "tlv was blocked from use because it is not from an identified developer". Click on the "Open Anyway" button.
    - Alternative. In Finder, go to the folder where TLV is. Right click the TLV app, then hold Shift and click "Open". In the dialog select to "Open Anyway"
5. Open TLV again, if everything went well, TLV should open without warnings now.

How do I build Tableau Log Viewer?
---------------
See [INSTALL.md](INSTALL.md)

Is Tableau Log Viewer supported?
---------------
Tableau Log Viewer is supported by the community. This is intended to be a self service tool and includes a user guide. Any requests or issues discovered should be filed in the [Issue Tracker](https://github.com/tableau/tableau-log-viewer/issues).

How can I contribute to Tableau Log Viewer?
---------------
Code contributions & improvements by the community are welcomed & encouraged! See the [LICENSE](LICENSE) file for current open-source licensing and use information. Also, if you are new to GitHub - read [this handy guide](https://guides.github.com/activities/contributing-to-open-source/) on how to get started!

A Word About Licenses
---------------
Tableau Log Viewer is released under MIT license however it relies on several other components:
* [Qt](https://www.qt.io/). We are using Qt's components that are licensed under [LGPLv3 license](https://www.qt.io/licensing-comparison/). Qt source code is located [here](http://code.qt.io/cgit/qt/qtbase.git/tree/) and instructions for obtaining it are located [here](https://wiki.qt.io/Building_Qt_5_from_Git#Getting_the_source_code). We will be keeping these links up to date and making sure they provide clear directions for obtaining Qt's source code in case you need it.
* [Query Graphs](https://github.com/tableau/query-graphs). This is our side project that is used for query tree visualization. It is released under the MIT license and uses the d3js library, released under the BSD license.
* [Solarized theme](https://github.com/altercation/solarized). This a collection of colors and guidelines for UI applications designed by Ethan Schoonover. The "Solarized Light" and "Solarized Dark" themes in TLV are using these color schemes. It is released under the MIT license.

