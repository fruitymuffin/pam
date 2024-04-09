# Parameters
# SRC_CS: The source C files to compie
# SRC_CPPS: The source CPP files to compile
# EXEC: The executable name

# Specify source files
BUILD_DIR := build
SRC_DIR := src

SRC_CPPS := $(wildcard $(SRC_DIR)/*.cpp)
EXEC     := $(BUILD_DIR)/pam

# Checks if source files exist
ifeq ($(SRC_CS) $(SRC_CPPS),)
  $(error No source files specified)
endif

ifeq ($(EXEC),)
  $(error No executable file specified)
endif

# Set c/c++ compilers
CC                  ?= gcc
CXX                 ?= g++

# This only sets PUREGEV_ROOT if not already set.
# /opt/jai/ebus_sdk/Ubuntu-22.04-x86_64/bin/set_puregev_env.sh should be run first to set it correctly!!
PUREGEV_ROOT        ?= ../../..
PV_LIBRARY_PATH      =$(PUREGEV_ROOT)/lib

# Compile Flags
CFLAGS              += -I$(PUREGEV_ROOT)/include -I.
CPPFLAGS            += -I$(PUREGEV_ROOT)/include -I. 
ifdef _DEBUG
    CFLAGS    += -g -D_DEBUG
    CPPFLAGS  += -g -D_DEBUG
else
    CFLAGS    += -O3
    CPPFLAGS  += -O3
endif
CFLAGS    += -D_UNIX_ -D_LINUX_ -fPIC -std=c++14
CPPFLAGS  += -D_UNIX_ -D_LINUX_ -DQT_GUI_LIB -fPIC -std=c++14

# Linker flags (link against all the ebus libs)
LDFLAGS             += -L$(PUREGEV_ROOT)/lib         \
                        -lPvAppUtils                 \
                        -lPtConvertersLib            \
                        -lPvBase                     \
                        -lPvBuffer                   \
                        -lPvGenICam                  \
                        -lPvStream                   \
                        -lPvDevice                   \
                        -lPvTransmitter              \
                        -lPvVirtualDevice            \
                        -lPvPersistence              \
                        -lPvSerial                   \
                        -lPvSystem                   \
                        -lPvCameraBridge



# Conditional linking and usage of the GUI on the sample only when available
ifneq ($(wildcard $(PUREGEV_ROOT)/lib/libPvGUI.so),)
    LDFLAGS   += -lPvGUI -lPvCodec
endif 

# Add simple imaging lib to the linker options only if available
ifneq ($(wildcard $(PUREGEV_ROOT)/lib/libSimpleImagingLib.so),)
    LDFLAGS   += -lSimpleImagingLib
endif

# Add CoreGEV lib to the linker options only if available
ifneq ($(wildcard $(PUREGEV_ROOT)/lib/libPvCoreGEV.so),)
    LDFLAGS   += -lPvCoreGEV
endif 

# Configure Genicam
GEN_LIB_PATH = $(PUREGEV_ROOT)/lib/genicam/bin/Linux64_x64
LDFLAGS      += -L$(GEN_LIB_PATH)

# Configure Qt compilation if any
SRC_MOC              =
MOC			         =
RCC					 =
FILES_QTGUI          = $(shell grep -l -d skip --exclude=Makefile Q_OBJECT *)

ifneq ($(wildcard /etc/redhat-release),)
    QMAKE = qmake-qt5
else ifneq ($(wildcard /etc/centos-release),)
    QMAKE = qmake-qt5
else
    QMAKE = qmake
endif

ifneq ($(FILES_QTGUI),)
    # This is a sample compiling Qt code
    HAVE_QT=$(shell which $(QMAKE) &>/dev/null ; echo $?)
    ifeq ($(HAVE_QT),1)
		# We cannot compile the sample without the Qt SDK!
 		$(error The sample $(EXEC) requires the Qt SDK to be compiled. See share/samples/Readme.txt for more details)
    endif

    # Query qmake to find out the folder required to compile
    QT_SDK_BIN        = $(shell $(QMAKE) -query QT_INSTALL_BINS)
    QT_SDK_LIB        = $(shell $(QMAKE) -query QT_INSTALL_LIBS)
    QT_SDK_INC        = $(shell $(QMAKE) -query QT_INSTALL_HEADERS)

    # We have a full Qt SDK installed, so we can compile the sample
    CFLAGS 	         += -I$(QT_SDK_INC) -I$(QT_SDK_INC)/QtCore -I$(QT_SDK_INC)/QtGui -I$(QT_SDK_INC)/QtWidgets
    CPPFLAGS         += -I$(QT_SDK_INC) -I$(QT_SDK_INC)/QtCore -I$(QT_SDK_INC)/QtGui -I$(QT_SDK_INC)/QtWidgets
    LDFLAGS          += -L$(QT_SDK_LIB) -lQt5Core -lQt5Gui -lQt5Widgets

    QT_LIBRARY_PATH   = $(QT_SDK_LIB)

    FILES_MOC            = $(shell grep -l Q_OBJECT *)
    ifneq ($(FILES_MOC),)
	    SRC_MOC           = $(FILES_MOC:%h=moc_%cxx)
	    FILES_QRC         = $(shell ls *.qrc)
	    SRC_QRC           = $(FILES_QRC:%qrc=qrc_%cxx)

	    OBJS             += $(SRC_MOC:%.cxx=%.o)
	    OBJS		     += $(SRC_QRC:%.cxx=%.o)

        MOC               = $(QT_SDK_BIN)/moc
  	    RCC               = $(QT_SDK_BIN)/rcc
    endif
endif

LD_LIBRARY_PATH       = $(PV_LIBRARY_PATH):$(QT_LIBRARY_PATH):$(GEN_LIB_PATH)
export LD_LIBRARY_PATH

OBJS      += $(SRC_CPPS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
OBJS      += $(SRC_CS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(EXEC)

clean:
	rm -rf $(SRC_MOC) $(SRC_QRC)
	@$(RM) -rv $(BUILD_DIR) # The @ disables the echoing of the command

moc_%.cxx: %.h
	$(MOC) $< -o $@

qrc_%.cxx: %.qrc
	$(RCC) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cxx
	$(CXX) -c $(CPPFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -c $(CPPFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

$(EXEC): $(BUILD_DIR) $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) 


.PHONY: all clean