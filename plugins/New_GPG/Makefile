CFLAGS=-g  -mdll -mwindows -I../miranda-im/miranda/include -I./ -I/home/sss/temp/windows/libs/utf8cpp/include -D DEBUG -D _UNICODE -D UNICODE -DBOOST_SYSTEM_STATIC_LINK=1 -DBOOST_THREAD_USE_LIB
CXXFLAGS=${CFLAGS}
LDFLAGS=-static-libgcc -Wl,-O1 -shared -s -Wl,--subsystem,windows,--kill-at -lshlwapi -lcomdlg32 -lboost_system-mt -lboost_thread_win32-mt -lboost_random-mt -lboost_date_time-mt
CPPFLAGS =
CC=i686-pc-mingw32-gcc
CXX=i686-pc-mingw32-g++
STRIP=i686-pc-mingw32-strip
LD=i686-pc-mingw32-ld
LNK_COMMON=-lkernel32

MAINOBJS=jabber_account.o clist.o init.o options.o gpg_wrapper.o srmm.o messages.o utilities.o metacontacts.o main.o log.o icons.o

WINDRES=i686-pc-mingw32-windres

all: main
main: $(MAINOBJS)
	$(WINDRES) -i new_gpg.rc -o resources.o
	$(CXX) $(MAINOBJS) resources.o $(LNK_COMMON) $(LDFLAGS) -o new_gpg.dll
	$(STRIP) new_gpg.dll
	upx --best --compress-icons=0 new_gpg.dll
clean:
	rm *.o
	rm new_gpg.dll
