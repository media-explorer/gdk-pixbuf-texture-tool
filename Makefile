#  gcc -shared -fPIC -Wno-write-strings `pkg-config --cflags --libs gdk-pixbuf-2.0` -I./PVRTexLib -o libpixbufloader-pvrtc.so gdk-pixbuf-pvrtc.cc PVRTexLib/libPVRTexLib.a -lstdc++

#& sudo cp libpixbufloader-pvrtc.so /usr/lib/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-pvrtc.so && sudo 

CFLAGS          := -g -fPIC -Wall -Wno-write-strings -Wno-sign-compare  $(shell pkg-config --cflags gdk-pixbuf-2.0)
INCLUDES        := -I./PVRTexLib
GDK_PIXBUF_LIBS := $(shell pkg-config --libs gdk-pixbuf-2.0)
LIBS            := PVRTexLib/libPVRTexLib.a -lstdc++ $(GDK_PIXBUF_LIBS)

INSTALL_DIR := $(shell pkg-config --variable=gdk_pixbuf_moduledir gdk-pixbuf-2.0)/

all: libpixbufloader-pvr.so gdk-pixbuf-texture-tool

libpixbufloader-pvr.so: gdk-pixbuf-pvr.cc
	gcc -shared $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

gdk-pixbuf-texture-tool: gdk-pixbuf-texture-tool.c
	gcc -o $@ $(CFLAGS) $^ $(GDK_PIXBUF_LIBS)

install: libpixbufloader-pvr.so
	cp $^ $(INSTALL_DIR)
	gdk-pixbuf-query-loaders-32 --update-cache

clean:
	rm -f libpixbufloader-pvr.so gdk-pixbuf-texture-tool

.PHONY: all install clean
