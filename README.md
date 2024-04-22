# Pam GUI
## 1. Installation
Currently using [eBUS SDK](https://www.jai.com/support-software/ubuntu-x86) for JAI version 6.4 for ubuntu 22.04 x86-64bit. The quickstart guide for linux in the above link has info for installation. Before installing eBUS SDK the following packages are required:
1. sudo apt-get install libavcodec58
2. sudo apt-get install qtbase5-dev qt5-qmake
3. sudo apt-get install build-essential python3-numpy

Ensure that gcc12.X is being used. Install the SDK.
4. sudo dpkg -i eBUS_SDK_<distribution_targeted>-<6.x.x>-<SDK build #>.deb

Before compiling any programs which link the eBUS libraries source the relevant environment variables:
```
source /opt/jai/ebus_sdk/Ubuntu-22.04-x86_64/bin/set_puregev_env.sh
```

NOTE - the performance of the camera was found to be significantly better when using the eBUS Universal Pro Driver. It is installed with the eBUS SDK. If the linux kernel is updated during routine updates the driver needs to be rebuilt.
To rebuild the driver:
```
cd /opt/jai/ebus_sdk/Ubuntu-22.04-x86_64/modules/
sudo ./build.sh
sudo ./install.sh --install
```

To check the status of the driver use:
```
./ebdriverlauncher.sh status
```

## 2. Compiling
The application can be compiled via make from the src/ directory, assuming all required environment variables are already sourced in the current terminal.
```
cd ~/pam/src
make
```
Alternatively it can be run via the build.sh script, which sources the relevant environment variables first.

## 3. Running
Run the executable in the build/ directory with:
```
./build/pam
```

## 4. Extras
The eBUS SDK suggests enabling jumbo network frames for best performance. Assuming the camera is connected to network adapter enps20, this is done by.
```
sudo ip link set enp2s0 mtu 9000
```
Where 9000 indicates a 9000 byte frame.

## 5. Application
1. The aqcuisition rate of the camera is an attribute that can be changed. However, actual received FPS may be different from the selected acquisition rate. The actual frame rate can be improved by reducing the image width/height settings which allows much higher frame rates. Changing the height/width does not scale the image but crops it.
2. For low light applications, the pixel binning option can improve the camera sensitivity significanly. The outcome is a brighter image at half the resolution and a greater possible frame rate.

# AVR Controller
The AVR controller is based on the AVR64DA32 chip which is programmed via the UPDI interface.

## 1. Compiling
The C/C++ code is compiled using avr-gcc/avr-g++ and makes use of avr-libc. [Microchip Studio](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio) was used as the IDE as it integrates the avr-g++ toolchain so building is easy.

## 2. Uploading
The serialUPDI interface was used to upload .hex or .elf files to the chip with the help of [avrdude](https://github.com/avrdudes/avrdude). An avrdude upload command can be added to Microchip Studio as a external tool to make uploading executables easier.
Go to Tools->External Tools and add a new tool. The tool was configured as:

Command:
```
C:\Program Files\avrdude-v7.3-windows-x64\avrdude.exe
```
Arguments:
```
-p avr64da32 -P COM5 -b 115200 -c serialupdi -C "C:\Program Files\avrdude-v7.3-windows-x64\avrdude.conf" -U flash:w:$(BinDir)\led-controller.hex:i
```

Initial Directory:
```
$(BinDir)
```

In order to set fuses with avrdude on the newer DA series chips, it was necessary to manually write it. Luckily the corrent fuse locations are specified in avrdude.conf under the DX family information. To enable the reset pin, the fuse was written by:
```
avrdude -c serialupdi -p avr64da32 -P com5 -U syscfg0:w:0xC8:m
```