#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc.exe
CCC=g++.exe
CXX=g++.exe
FC=gfortran.exe
AS=

# Macros
CND_PLATFORM=MinGW-Windows
CND_CONF=Debug
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/alphablend.o \
	${OBJECTDIR}/init.o \
	${OBJECTDIR}/contact.o \
	${OBJECTDIR}/cluiservices.o \
	${OBJECTDIR}/Docking.o \
	${OBJECTDIR}/clcmsgs.o \
	${OBJECTDIR}/wallpaper.o \
	${OBJECTDIR}/forkthread.o \
	${OBJECTDIR}/clcopts.o \
	${OBJECTDIR}/clistmod.o \
	${OBJECTDIR}/clistevents.o \
	${OBJECTDIR}/clistmenus.o \
	${OBJECTDIR}/clcitems.o \
	${OBJECTDIR}/CLUIFrames/groupmenu.o \
	${OBJECTDIR}/CLUIFrames/movetogroup.o \
	${OBJECTDIR}/extBackg.o \
	${OBJECTDIR}/clcutils.o \
	${OBJECTDIR}/coolsb/coolscroll.o \
	${OBJECTDIR}/CLUIFrames/cluiframes.o \
	${OBJECTDIR}/coolsb/coolsblib.o \
	${OBJECTDIR}/clc.o \
	${OBJECTDIR}/CLCButton.o \
	${OBJECTDIR}/clcpaint.o \
	${OBJECTDIR}/CLUIFrames/framesmenu.o \
	${OBJECTDIR}/statusfloater.o \
	${OBJECTDIR}/clistsettings.o \
	${OBJECTDIR}/clistopts.o \
	${OBJECTDIR}/viewmodes.o \
	${OBJECTDIR}/clisttray.o \
	${OBJECTDIR}/cluiopts.o \
	${OBJECTDIR}/statusbar.o \
	${OBJECTDIR}/clui.o \
	${OBJECTDIR}/rowheight_funcs.o \
	${OBJECTDIR}/clcidents.o \
	${OBJECTDIR}/clnplus.o \
	${OBJECTDIR}/commonheaders.o

# C Compiler Flags
CFLAGS=-m32

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lgdi32 -lcomctl32 -lcomdlg32 -lmsvcrt -lkernel32 -lmsimg32 -lshlwapi -luser32 -lshell32 -lshlwapi -luser32

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Debug.mk dist/Debug/MinGW-Windows/libclist_nicer.dll

dist/Debug/MinGW-Windows/libclist_nicer.dll: ${OBJECTFILES}
	${MKDIR} -p dist/Debug/MinGW-Windows
	${LINK.cc} -shared -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libclist_nicer.dll -s ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/alphablend.o: nbproject/Makefile-${CND_CONF}.mk alphablend.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/alphablend.o alphablend.c

${OBJECTDIR}/init.o: nbproject/Makefile-${CND_CONF}.mk init.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/init.o init.c

${OBJECTDIR}/contact.o: nbproject/Makefile-${CND_CONF}.mk contact.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/contact.o contact.c

${OBJECTDIR}/cluiservices.o: nbproject/Makefile-${CND_CONF}.mk cluiservices.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/cluiservices.o cluiservices.c

${OBJECTDIR}/Docking.o: nbproject/Makefile-${CND_CONF}.mk Docking.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/Docking.o Docking.c

${OBJECTDIR}/clcmsgs.o: nbproject/Makefile-${CND_CONF}.mk clcmsgs.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clcmsgs.o clcmsgs.c

${OBJECTDIR}/wallpaper.o: nbproject/Makefile-${CND_CONF}.mk wallpaper.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/wallpaper.o wallpaper.c

${OBJECTDIR}/forkthread.o: nbproject/Makefile-${CND_CONF}.mk forkthread.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/forkthread.o forkthread.c

${OBJECTDIR}/clcopts.o: nbproject/Makefile-${CND_CONF}.mk clcopts.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clcopts.o clcopts.c

${OBJECTDIR}/clistmod.o: nbproject/Makefile-${CND_CONF}.mk clistmod.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clistmod.o clistmod.c

${OBJECTDIR}/clistevents.o: nbproject/Makefile-${CND_CONF}.mk clistevents.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clistevents.o clistevents.c

${OBJECTDIR}/clistmenus.o: nbproject/Makefile-${CND_CONF}.mk clistmenus.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clistmenus.o clistmenus.c

${OBJECTDIR}/clcitems.o: nbproject/Makefile-${CND_CONF}.mk clcitems.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clcitems.o clcitems.c

${OBJECTDIR}/CLUIFrames/groupmenu.o: nbproject/Makefile-${CND_CONF}.mk CLUIFrames/groupmenu.c 
	${MKDIR} -p ${OBJECTDIR}/CLUIFrames
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/CLUIFrames/groupmenu.o CLUIFrames/groupmenu.c

${OBJECTDIR}/CLUIFrames/movetogroup.o: nbproject/Makefile-${CND_CONF}.mk CLUIFrames/movetogroup.c 
	${MKDIR} -p ${OBJECTDIR}/CLUIFrames
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/CLUIFrames/movetogroup.o CLUIFrames/movetogroup.c

${OBJECTDIR}/extBackg.o: nbproject/Makefile-${CND_CONF}.mk extBackg.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/extBackg.o extBackg.c

${OBJECTDIR}/clcutils.o: nbproject/Makefile-${CND_CONF}.mk clcutils.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clcutils.o clcutils.c

${OBJECTDIR}/coolsb/coolscroll.o: nbproject/Makefile-${CND_CONF}.mk coolsb/coolscroll.c 
	${MKDIR} -p ${OBJECTDIR}/coolsb
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/coolsb/coolscroll.o coolsb/coolscroll.c

${OBJECTDIR}/CLUIFrames/cluiframes.o: nbproject/Makefile-${CND_CONF}.mk CLUIFrames/cluiframes.c 
	${MKDIR} -p ${OBJECTDIR}/CLUIFrames
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/CLUIFrames/cluiframes.o CLUIFrames/cluiframes.c

${OBJECTDIR}/coolsb/coolsblib.o: nbproject/Makefile-${CND_CONF}.mk coolsb/coolsblib.c 
	${MKDIR} -p ${OBJECTDIR}/coolsb
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/coolsb/coolsblib.o coolsb/coolsblib.c

${OBJECTDIR}/clc.o: nbproject/Makefile-${CND_CONF}.mk clc.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clc.o clc.c

${OBJECTDIR}/CLCButton.o: nbproject/Makefile-${CND_CONF}.mk CLCButton.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/CLCButton.o CLCButton.c

${OBJECTDIR}/clcpaint.o: nbproject/Makefile-${CND_CONF}.mk clcpaint.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clcpaint.o clcpaint.c

${OBJECTDIR}/CLUIFrames/framesmenu.o: nbproject/Makefile-${CND_CONF}.mk CLUIFrames/framesmenu.c 
	${MKDIR} -p ${OBJECTDIR}/CLUIFrames
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/CLUIFrames/framesmenu.o CLUIFrames/framesmenu.c

${OBJECTDIR}/statusfloater.o: nbproject/Makefile-${CND_CONF}.mk statusfloater.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/statusfloater.o statusfloater.c

${OBJECTDIR}/clistsettings.o: nbproject/Makefile-${CND_CONF}.mk clistsettings.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clistsettings.o clistsettings.c

${OBJECTDIR}/clistopts.o: nbproject/Makefile-${CND_CONF}.mk clistopts.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clistopts.o clistopts.c

${OBJECTDIR}/viewmodes.o: nbproject/Makefile-${CND_CONF}.mk viewmodes.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/viewmodes.o viewmodes.c

${OBJECTDIR}/clisttray.o: nbproject/Makefile-${CND_CONF}.mk clisttray.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clisttray.o clisttray.c

${OBJECTDIR}/cluiopts.o: nbproject/Makefile-${CND_CONF}.mk cluiopts.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/cluiopts.o cluiopts.c

${OBJECTDIR}/statusbar.o: nbproject/Makefile-${CND_CONF}.mk statusbar.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/statusbar.o statusbar.c

${OBJECTDIR}/clui.o: nbproject/Makefile-${CND_CONF}.mk clui.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clui.o clui.c

${OBJECTDIR}/rowheight_funcs.o: nbproject/Makefile-${CND_CONF}.mk rowheight_funcs.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/rowheight_funcs.o rowheight_funcs.c

${OBJECTDIR}/clcidents.o: nbproject/Makefile-${CND_CONF}.mk clcidents.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clcidents.o clcidents.c

${OBJECTDIR}/clnplus.o: nbproject/Makefile-${CND_CONF}.mk clnplus.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/clnplus.o clnplus.cpp

${OBJECTDIR}/commonheaders.o: nbproject/Makefile-${CND_CONF}.mk commonheaders.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -s -D__GNUWIN32__ -I../../include -I../../include/mingw -MMD -MP -MF $@.d -o ${OBJECTDIR}/commonheaders.o commonheaders.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/Debug
	${RM} dist/Debug/MinGW-Windows/libclist_nicer.dll

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
