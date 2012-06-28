!IF "$(CFG)" == ""
CFG=Release
#CFG=Debug

!MESSAGE No configuration specified. Defaulting to $(CFG)
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
LINK32=link.exe

out=../../bin/Debug/Plugins
obj=obj
LIBS = user32.lib netapi32.lib comdlg32.lib shlwapi.lib


LINK32_OBJS= \
	"$(obj)\resource.res" \
	"$(obj)\wumfplug.obj" \
	"$(obj)\list.obj" \
	"$(obj)\resizer.obj" \
	"$(obj)\wumf.obj"


all : "$(out)\wumfplug.dll" test

$(obj):
	if not exist $(obj) mkdir $(obj)


$(out):
	if not exist "$(out)/$(NULL)" mkdir "$(out)"

!IF  "$(CFG)" == "Release"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WIN32_WINNT" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fp"$(obj)/wumfplug.pch" /YX /Fo"$(obj)/" /Fd"$(obj)/" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(obj)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(obj)\wumfplug.bsc" 
BSC32_SBRS= \
	
LINK32_FLAGS= $(LIBS) /nologo /dll /incremental:no /pdb:"$(obj)\wumfplug.pdb" /machine:I386 /out:"$(out)/wumfplug.dll" /implib:"$(obj)/wumfplug.lib" 

!ELSEIF  "$(CFG)" == "Debug"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_WIN32_WINNT" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fp"$(obj)\wumfplug.pch" /YX /Fo"$(obj)\\" /Fd"$(obj)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(obj)/resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(obj)/wumfplug.bsc" 
BSC32_SBRS= \
	
LINK32_FLAGS=$(LIBS) /nologo /dll /incremental:yes /pdb:"$(obj)\wumfplug.pdb" /debug /machine:I386 /out:"$(out)\wumfplug.dll" /implib:"$(obj)/wumfplug.lib" /pdbtype:sept 

!ENDIF 

"$(out)\wumfplug.dll" : "$(out)" $(DEF_FILE) $(LINK32_OBJS)
	$(LINK32) @<<
	$(LINK32_FLAGS) $(LINK32_OBJS)
<<


.c{$(obj)}.obj::
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

.cpp{$(obj)}.obj::
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

.cxx{$(obj)}.obj::
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

.c{$(obj)}.sbr::
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

.cpp{$(obj)}.sbr::
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

.cxx{$(obj)}.sbr::
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

$(obj)/wumfplug.obj: wumfplug.c $(obj) wumf.h

$(obj)/wumf.obj: wumf.c $(obj) wumf.h

$(obj)/resizer.obj: resizer.c $(obj) wumf.h


$(obj)/resource.res: resource.rc $(obj)
	$(RSC) $(RSC_PROJ) resource.rc

clean:
	rm -f $(obj)/*.* $(out)/wumfplug.* *.aps 

yPER: $(out)/wumfplug.dll
	yPER $(out)/wumfplug.dll /REALIGN_NORMAL

distr: zip

zip: $(out)/wumfplug.dll yPER res/wumf.readme.txt
	pkzip25 -add -max wumf.zip $(out)/wumfplug.dll res/wumf.readme.txt

src:
	rar a -ag wumf-src.rar *.c *.h *.hm *.txt Makefile *.rc res/*.*

re: clean all

test:
	"C:\Documents and Settings\Nike.Neosoft\Application Data\Microsoft\Internet Explorer\Quick Launch\miranda32dbg.exe.lnk"
