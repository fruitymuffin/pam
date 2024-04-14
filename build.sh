# Set environment variable for lib directory
source /opt/jai/ebus_sdk/Ubuntu-22.04-x86_64/bin/set_puregev_env.sh

# enable jumbo frames
sudo ip link set eth1 mtu 9000

# Makefile must specify
# SRC_CPPS
# EXEC

make
