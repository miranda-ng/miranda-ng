all:
	i686-pc-mingw32-gcc -c -DBUILD_DLL -D UNICODE -D _UNICODE *.c -I../../include -I/usr/i686-pc-mingw32/usr/include -I. -w -mwin32 -mwindows -mdll -march=i686 -msse -O2 -pipe
	i686-pc-mingw32-windres -i watrack_mpd.rc -o resources.o
	i686-pc-mingw32-gcc -shared -o watrack_mpd.dll *.o -Wl,-O1,-s
	upx -9 watrack_mpd.dll

clean:
	rm *.o

clean-all: 
	rm *.o *.dll
	
