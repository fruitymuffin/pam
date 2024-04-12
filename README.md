# Pam

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