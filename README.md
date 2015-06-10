msr.kext
========
OS X kernel extension for reading MSRs (Model Specific Registers). This is a
companion project to [µProcessor](https://github.com/relan/uproc).

Building
--------
[Xcode](https://developer.apple.com/xcode/) is required. Use the following
command to build the kernel extension (kext):

    xcodebuild -project msr.xcodeproj

Usage
-----
Note that since OS X 10.10 each kext must have a valid digital signature.
Obviously kernel extensions you build do not have one. You need to enable the
developer mode:

    sudo nvram boot-args=kext-dev-mode=1

After reboot you will be able to load the kext:

    sudo chown -R root:wheel build/Release/msr.kext
    sudo kextutil build/Release/msr.kext
    sudo chown -R `id -un`:`id -gn` build/Release/msr.kext

The owner needs to be changed because `kextutil` requires it to be `root:wheel`.
After kext is loaded the owner can be changed back to current user.
