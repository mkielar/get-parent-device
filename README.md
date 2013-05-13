# get-parent-device

This program finds `Device Instance ID` of a *parent* of a Device given by
another `Device Instance ID`. The main reason to create this program was to be
able to obtain *"bus relation"* of `Win32_PnPEntity` objects connected to
`Win32_UsbController`, because plain WMI cannot do that.

For the details see the source of the [`main.cpp`][main.cpp] file.

#### Dependencies & build instructions

This program compiled without problems with `Visual Studio 2012 Express Edition for Desktop` and `Windows Driver Kit 7600.16385.1` installed.

The `Windows Driver Kit` is required, because program uses [`SetupAPI` functions][SetupAPI] which are a part of `Windows Driver Kit`.

#### Running

To run, the program requires [MSVC Redistributable][MSVCR] Package installed, or at least, requires the following files from 
`c:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\redist\x86\Microsoft.VC110.CRT` to be distributed along with the executable:

 * msvcp110.dll 
 * msvcr110.dll 
 * vccorlib110.dll 

#### Usage

        get-parent-device.exe DII PATTERN

#### Arguments:

        DII     - Device Instance ID of the Device whose parent is to be found
        PATTERN - Regular expression to match Parent's Device Instance ID

#### Examples:

 1. Get immediate parent:

        get-parent-device.exe "USBSTOR\DISK&VEN_GENERIC&PROD_STORAGE_DEVICE&REV_0207\000000000207&0" ".*"

    In this case the `".*"` will cause first found parent to be returned.

 2. Get usb hub the device is connected to:

        get-parent-device.exe "USBSTOR\DISK&VEN_GENERIC&PROD_STORAGE_DEVICE&REV_0207\000000000207&0" ".*\\ROOT_HUB.*"

    The program will search "up" the device tree until it finds a parent with a matching `Device Instance ID`.

[main.cpp]: https://github.com/mkielar/get-parent-device/blob/master/get-parent-device/main.cpp

[SetupAPI]: http://msdn.microsoft.com/en-us/library/windows/hardware/ff550855%28v=vs.85%29.aspx

[MSVCR]: http://www.microsoft.com/en-us/download/details.aspx?id=30679