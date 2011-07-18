#  gcc -shared -fPIC -Wno-write-strings `pkg-config --cflags --libs gdk-pixbuf-2.0` -I./PVRTexLib -o libpixbufloader-pvrtc.so gdk-pixbuf-pvrtc.cc PVRTexLib/libPVRTexLib.a -lstdc++

#& sudo cp libpixbufloader-pvrtc.so /usr/lib/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-pvrtc.so && sudo 

CFLAGS   := -g -fPIC -Wall -Wno-write-strings -Wno-sign-compare  $(shell pkg-config --cflags gdk-pixbuf-2.0)
INCLUDES := -I./PVRTexLib
LIBS     := PVRTexLib/libPVRTexLib.a -lstdc++ $(shell pkg-config --libs gdk-pixbuf-2.0)

INSTALL_DIR := $(shell pkg-config --variable=gdk_pixbuf_moduledir gdk-pixbuf-2.0)/

all: libpixbufloader-pvr.so

libpixbufloader-pvr.so: gdk-pixbuf-pvr.cc
	gcc -shared $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

install: libpixbufloader-pvr.so
	cp $^ $(INSTALL_DIR)
	gdk-pixbuf-query-loaders-32 --update-cache

clean:
	rm -f libpixbufloader-pvr.so

.PHONY: all install clean
