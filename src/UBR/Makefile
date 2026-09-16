ifeq ($(ARCH), 32)
MSYSTEM = MINGW32
endif

ifeq ($(MSYSTEM), MINGW32)
ARCH = 32
else
ARCH = 64
endif

ENABLE_DEBUG ?= 1
WXDIR = /home/Administrator/wxWidgets-3.2.10
WXINCLUDE = $(WXDIR)/include

ifeq ($(ARCH), 32)
WXLIB = $(WXDIR)/build-release/lib
else
WXLIB = $(WXDIR)/build-release64/lib
endif

EXTDIR = /home/Administrator/extlib

SRCS = App.cpp BaseFrame.cpp FrameImpl.cpp Misc.cpp MiscWx.cpp tstring.cpp\
       PrivilegeControl.cpp LZ4Stream.cpp\
       Widgets/DiskInfoPanel.cpp Widgets/DiskPanelImpl.cpp Widgets/RoundedPanel.cpp \
       Widgets/PrevNextPanel.cpp Widgets/ProgressPanel.cpp Widgets/SimpleButton.cpp\
       Worker/Events.cpp Worker/Clone.cpp Worker/Restore.cpp Worker/SectorTask.cpp\
       Worker/Backup.cpp\
       FileSystem/Crc32.cpp FileSystem/DiskInfo.cpp FileSystem/ForensicAnalysis.cpp \
       FileSystem/Fs.cpp FileSystem/Fs_fat.cpp FileSystem/Fs_ntfs.cpp FileSystem/MbrGpt.cpp\
       FileSystem/PhysicalDiskInfo.cpp FileSystem/VDisk_physical.cpp FileSystem/VDisk_raw.cpp\
       FileSystem/VDisk_vhd.cpp FileSystem/VDisk_vhdx.cpp FileSystem/VDisk_vss.cpp\
       FileSystem/VDiskFactory.cpp FileSystem/VDiskStream.cpp\
       FileSystem/VDisk_vhdx_wx.cpp FileSystem/VDisk_vhd_wx.cpp FileSystem/VDisk_raw_wx.cpp \
       VSS/Vss.cpp
RESOURCES = file.rc icon.rc version.rc manifest.rc

OBJS = $(SRCS:.cpp=.o)
RESOURCE_OBJS = $(RESOURCES:.rc=.o)

CFLAGS = -I./ -I$(EXTDIR)/include -I$(WXINCLUDE) \
         -std=c++11 -Wno-deprecated\
         -D_FILE_OFFSET_BITS=64  -D__WXMSW__  -D_UNICODE -DUNICODE -D__MINGW_USE_VC2005_COMPAT -D_WINDOWS

LFLAGS = -L$(WXLIB) -mwindows -mthreads -static -static-libgcc\
         -Wl,--major-os-version=6 -Wl,--minor-os-version=1 -Wl,--major-subsystem-version=6 -Wl,--minor-subsystem-version=1

ifeq ($(ARCH), 32)
	LFLAGS += -L$(EXTDIR)/lib
	CFLAGS += -I$(WXDIR)/build-release/lib/wx/include/msw-unicode-static-3.2
else
	LFLAGS += -L$(EXTDIR)/lib64
	CFLAGS += -I$(WXDIR)/build-release64/lib/wx/include/msw-unicode-static-3.2
endif

ifeq ($(ENABLE_DEBUG), 1)
	CFLAGS += -DDEBUG -g -O0
else
	CFLAGS += -DNDEBUG
	LFLAGS += -s
endif



LIBS =          -lwx_mswu_core-3.2 \
                -lwx_baseu-3.2 \
                -lwxjpeg-3.2 -lwxscintilla-3.2 -lwxpng-3.2 -lwxtiff-3.2 -lrpcrt4 \
                -lpcre2-16 \
                -loleaut32 -lole32 -luuid -luxtheme -lwinspool -lshell32 -lshlwapi -lcomctl32 \
                -lcomdlg32 -ladvapi32 -lversion -lws2_32 -lgdi32 -loleacc -lwinhttp -lkernel32 -luser32\
                -lsetupapi -lz -lvssapi -llz4

CC = g++
ifeq ($(ARCH), 32)
TARGET = UBR32.exe
else
TARGET = UBR64.exe
endif

$(TARGET): $(OBJS) $(RESOURCE_OBJS)
	$(CC) $(OBJS) $(RESOURCE_OBJS) $(LFLAGS) $(LIBS) -o $(TARGET)

$(OBJS): %.o:%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(RESOURCE_OBJS): %.o:%.rc
	windres.exe -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET) $(RESOURCE_OBJS)


