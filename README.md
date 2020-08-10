# qwtw 
another qwt-based 2D plotting library

![](readme/p1.png "plot examples")
![](readme/p2.png "plot description")
![](readme/p3.png "plot examples #2")

This is a [library header file](https://github.com/ig-or/qwtw/blob/master/qwtw/c_lib/qwtw.h)

And this is a [small example how to use it](https://github.com/ig-or/qwtw/blob/master/qwtw/qwtwtest.cpp)

This is a small page with description of some useful features of this library: https://github.com/ig-or/qwtw/wiki/qwtw-library-features


Also, there is a `Julia language` package as a wrapper for this library: https://github.com/ig-or/qwtwplot - so you can create all the plots from inside Julia Language.

If you need something in this library which is not implemented yet, feel free to implement it! %) Or at least write me about it - may be I'll be able to add the features you need.



how to install it:
==========================
Currently, works only for Windows x64. 

### Windows installation instructions:

#### short instructions

`vcpkg install  --triplet x64-windows qwtw`

#### long instructions
 * install `vcpkg` if you have not done it already:
 ```
 git clone https://github.com/microsoft/vcpkg.git
 cd vcpkg
 bootstrap-vcpkg.bat
 ```
 * `vcpkg install --triplet x64-windows qwtw`
 * wait for some time. in case "install from zero" it will download and build a few libraries and thier dependencies. Main libraries: QT, BOOST, QWT, Marble. This can take some time. 
 * `add <VCPKG_ROOT>/installed/x64-windows/bin` to `PATH`, if you need all those QT-related libraries be in your path. Be careful with this.
 * add `VCPKG_ROOT` environment variable, pointing to vcpkg.exe path. This is really helpful.


## Linux installation instructions (not working yet, anyway.. will eventually make all this work via vcpkg)
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
