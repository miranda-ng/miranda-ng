CFLAGS=-Os -fomit-frame-pointer -pipe -march=i686 -mdll -mwin32 -mwindows -I../miranda-im/miranda/include -I/home/sss/temp/windows/libs/utf8cpp/include -D UNICODE -D _UNICODE -DBUILD_DLL 
CXXFLAGS=${CFLAGS}
LDFLAGS=-shared -Wl,--subsystem,windows,--kill-at
CPPFLAGS =
CC=i686-pc-mingw32-gcc
CXX=i686-pc-mingw32-g++
STRIP=i686-pc-mingw32-strip
WINDRES=i686-pc-mingw32-windres
LD=i686-pc-mingw32-ld
LNK_COMMON=-lkernel32 -lboost_thread_win32-mt -lboost_system-mt -lboost_random-mt -lboost_regex-mt
MAINOBJS=eventhooker.o init.o options.o utilities.o stopspam.o
RESOURCES=stopspam.rc

all: main
main: $(MAINOBJS)
	$(WINDRES) -i $(RESOURCES) -o stopspam_res.o
	$(CXX) $(MAINOBJS) stopspam_res.o $(LNK_COMMON) $(LDFLAGS) -o stopspam.dll
	$(STRIP) stopspam.dll
#	upx -9 stopspam.dll
clean:
	rm *.o
	rm stopspam.dll
					
	
