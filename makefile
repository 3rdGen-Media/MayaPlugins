#############################################################################
# Makefile for Mac OS Lib (X)
#############################################################################

# Define here the name of your project's main executable, and the name of the
# build directory where the generated binaries and resources will go.

NAME	   = polyAbcExporter
SOURCEDIR = Maya
OUTNAME    = polyAbcExporter.bundle
OUTDIR  = ./

# Define here the minimal iOS version's MAJOR number (iOS3, iOS4 or iOS5)
#IOSMINVER = 5

# List here your project's resource files. They can be files or directories.
#RES = Info.plist icon.png

# Define here the compile options and the linker options for your project.
#CFLAGS  = -W -Wall -O2
CFLAGS =  -Wall -Wextra -std=c++1y#-c -framework CoreFoundation -framework CoreGraphics -framework OpenGL -framework CoreVideo -framework IOKit
LDFLAGS = -L/usr/local/lib -lAlembic -lImath -lHalf -lIex -L/Applications/Autodesk/maya2019/Maya.app/Contents/MacOS -lFoundation -lOpenMaya#-lhdf5 -lhdf5_hl#../../../../../System/Library/Frameworks/IOKit.framework/Versions/A/Resources/BridgeSupport
IFLAGS = -I$(MAYA_PATH)/include -I$(MAYA_PATH)/include/maya -I/usr/local/include/ -I/usr/local/include/Alembic/ -I/usr/local/include/OpenEXR
#LIBS = -IOKit

#############################################################################
# Except for specific projects, you shouldn't need to change anything below
#############################################################################

# Define which compiler to use and what are the mandatory compiler and linker
# options to build stuff for iOS. Here, use the ARM cross-compiler for iOS,
# define IPHONE globally and link against all available frameworks.
CC  = clang++ -bundle -arch x86_64 -stdlib=libc++
CFLAGS  += -DMAC_PLUGIN -DOSMac_ -DOSMac_MachO_# -ccc-host-triple arm-apple-darwin -march=armv6 --sysroot ../../usr/local/sys -integrated-as -fdiagnostics-format=msvc -fconstant-cfstrings -DIPHONE -D__IPHONE_OS_VERSION_MIN_REQUIRED=$(IOSMINVER)0000
#LDFLAGS += -lstdc++ $(addprefix -framework , $(notdir $(basename $(wildcard /Frameworks/iOS$(IOSMINVER)/*))))


HEADERS = polyAbcExporter/AbcExportInterface.h \
			polyAbcExporter/polyWriter.h \
			polyAbcExporter/polyAbcWriter.h \
			polyAbcExporter/polyExporter.h \
			polyAbcExporter/polyAbcExporter.h
					
SOURCE = polyAbcExporter/polyWriter.cpp \
			polyAbcExporter/polyAbcWriter.cpp \
			polyAbcExporter/polyExporter.cpp \
			polyAbcExporter/polyAbcExporter.cpp
						
OBJECTS = polyWriter.o \
			polyAbcWriter.o \
			polyExporter.o \
			polyAbcExporter.o
			
*.o:
	#cc -flat_namespace -bundle -undefined suppress \
  	#-o libanswer.bundle answer.o
	$(CC) $(CFLAGS) $(IFLAGS) $(LDFLAGS) -o $(OUTNAME) $(SOURCE)
	#$(CC) $(LDFLAGS) -o $(OUTNAME) $(OBJECTS) 
	#$(CC) $(CFLAGS) $(IFLAGS) $(LDFLAGS) $(SOURCE) -o $(OUTNAME)
	#ar -rs $(OUTDIR)/$(OUTNAME) $(OBJECTS)
	#/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar -rs $(OUTDIR)/$(OUTNAME) $(OBJECTS)
	#/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib -c $(OUTDIR)/$(OUTNAME)
	#libtool -c -static -o $(OUTDIR)/$(OUTNAME) $(OBJECTS)
	#cp -rvf $(SOURCEDIR)/*.h ../include/CoreRender/$(NAME)
	#cp -rvf $(NAME).h ../include/CoreRender
	#mkdir -p ../bin/x86_64
	#cp -rvf $(OUTNAME) ../bin/x86_64

all: $(NAME)

$(NAME):
	#$(CC) $(CFLAGS) $(IFLAGS) $(LDFLAGS) $(SOURCE) $(OUTDIR)/$(OUTNAME)
	$(CC) $(CFLAGS) $(IFLAGS) $(LDFLAGS) $(SOURCE)
	ar -rv $(OUTDIR)/$(OUTNAME) $(OBJECTS)
	#ar -rv $(OUTDIR)/$(OUTNAME) $(OBJECTS)
	#cp -rvf $(SOURCEDIR)/*.h ../include/CoreRender/$(NAME)
	#cp -rvf $(NAME).h ../include/CoreRender
	#mkdir -p ../bin/x86_64
	#cp -rvf $(OUTNAME) ../bin/x86_64

clean:
	rm -f *.o $(OUTNAME)
	
