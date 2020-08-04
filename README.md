# qwtw 
another qwt-based 2D plotting library

![](readme/p1.png "plot examples")
![](readme/p2.png "plot description")
![](readme/p3.png "plot examples #2")

This is a [library header file](https://github.com/ig-or/qwtw/blob/master/qwtw/c_lib/qwtw_c.h)

And this is a [small example how to use it](https://github.com/ig-or/qwtw/blob/master/qwtw/c_lib/qwtwtest.cpp)

This is a small page with description of some useful features of this library: https://github.com/ig-or/QWTwPlot.jl/wiki/qwtw-library-features


Also, there is a `Julia language` package as a wrapper for this library: https://github.com/ig-or/qwtwplot - so you can create all the plots from inside Julia Language.

If you need something in this library which is not implemented yet, feel free to implement it! %) Or at least write me about it - may be I'll be able to add the features you need.



how to install it:
==========================
Currently, this library was tested for `Windows` and for `Linux` (looks like working!)

## Windows installation instructions:

May be you can use an installer. All the installers are located here:
		https://github.com/ig-or/qwtw/releases

Currently there is an installer for Windows X64 bit. Tested on Windows7 x64; If you have Windows10 x64 - please try it, may be it will work?

If you will not use an installer, you can build everything yourself.  How to do this: instructions below. Please note that this description its not perfect (at least "plot on a map" feature may be not working). But as a small reference (required libraries list) you can see also an `InnoSetup script` from here https://github.com/ig-or/qwtw/blob/master/setup/inno-setup-script.iss


* install BOOST (v1.60), QT (5.6.1), CMAKE, python 2.7, jom package, Visual Studio (12?)
* all those packages (mentioned above) should 'coincide' with each other: for example
  QT5 and BOOST binaries should correspond to your VisualStudio version, and system architecture (windows version and 32 or 64 bit)
* go to Visual Studio Command prompt
* make all the DLLs be in your PATH (DLL from Boost, QT, etc) (something like `set PATH=my-qt-dir\bin`); I also recommend to make all the operations from this `Visual Studio Command prompt`
* download `QWT` package (I used v6.1.2); may be bigger version can also work?; Build QWT with your VisualStudio compiler, linking with installed QT libraries
* download `Marble` package  and build it with your VisualStudio compiler, linking with installed QT libraries; Follow instructions from here: https://marble.kde.org/sources.php ; pay attention to use `QT` mode (not `KDE`)
* `git clone https://github.com/ig-or/qwtw.git qwtw`
* `cd qwtw\c_lib`
* `mkdir build`
* `cd build`
* `mkdir release`
* `cd release`
* Build qwtw library
 * `cmake -G "NMake Makefiles JOM" -DCMAKE_BUILD_TYPE=Release ../../.`
 * or `cmake -G "Visual Studio 12 2013 Win64" ../../.`
* `jom -j 9` (or use VisualStudio)
* cd ..\..\..\..\
* cd lib\release
* try to run the test: `qwtwtest.exe`; when running the test, remember that Windows should be able to find all the dependent DLLs (quite a lot of DLLs).

library test should start (create a few very simple plots)


## Linux installation instructions
May be you can use an installer. The installer for Fedora24-x64 can be found here: 	https://github.com/ig-or/qwtw/releases ; you can install this RPM with following command: `sudo dnf install qwtw-binary-rpm-package-name.rpm`

Or, you can create your own RPM using qwtw-version-number-src.rpm and install it.

If you will not use an installer, you  can build everything by youself - much easier compared to Windows. Should be no problem at all.

I tested following procedure it on "empty" Fedora24 x64 installation, and everything worked.

*  we need latest cmake

       sudo dnf install cmake
* we need c++

        sudo dnf install gcc-c++
* QT 5

        sudo dnf install -y `dnf search Qt5 | grep devel | grep x86_64 | grep '^qt5' | awk '{print $1}'`
        PATH=/usr/lib64/qt5/bin:$PATH
* Marble package:

        sudo dnf install -y marble-astro-devel.x86_64  marble-widget-qt5-devel.x86_64
* QWT:

        dnf install -y `dnf search qwt | grep qt5 | grep x86_64 | awk '{print $1}'`
* get the repo:

        git clone https://github.com/ig-or/qwtw.git qwtw
* build everything:

        cd qwtw/qwtw/c_lib
        mkdir build
        cd build
        mkdir release
        cd release
        cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr  ../../.
        make
		make install
        cd ../../../../lib/release
* run test example:

        ./qwtwtest





Authors: Igor Sandler and Anton Klimenkov
