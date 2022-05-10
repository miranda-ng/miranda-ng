##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=mir_core
ConfigurationName      :=Debug
WorkspaceConfiguration :=Debug
WorkspacePath          :=/home/ghazan/miranda-ng/codelite
ProjectPath            :=/home/ghazan/miranda-ng/src/mir_core
IntermediateDirectory  :=../../codelite/obj/debug/$(ProjectName)
OutDir                 :=$(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=George Hazan
Date                   :=10/05/22
CodeLitePath           :=/home/ghazan/.codelite
MakeDirCommand         :=mkdir -p
LinkerName             :=g++
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputDirectory        :=/home/ghazan/miranda-ng/codelite/build-$(WorkspaceConfiguration)/lib
OutputFile             :=../../codelite/build-$(WorkspaceConfiguration)/lib/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
LinkOptions            :=  $(shell wx-config   --libs --unicode=yes)
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../../include $(IncludeSwitch)"`pkg-config --cflags --libs elementary`" 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overridden using an environment variable
##
AR       := ar rcus
CXX      := g++
CC       := gcc
CXXFLAGS :=  -g $(shell wx-config --cxxflags   --unicode=yes) -fPIC $(Preprocessors)
CFLAGS   :=  -g $(shell wx-config --cxxflags   --unicode=yes) -fPIC $(Preprocessors)
ASFLAGS  := 
AS       := as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/src_logger.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_md5.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_mstring.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_db.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Linux_CDlgBase.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_stdafx.cxx$(ObjectSuffix) $(IntermediateDirectory)/src_http.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_tinyxml2.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_sha256.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_utf.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/src_Linux_strutil.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_utils.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_memory.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Linux_CDbLink.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Linux_CCtrlBase.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_tinyxml2_utils.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Linux_fileutil.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_lists.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_bitmaps.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_binbuffer.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/src_sha1.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) "$(IntermediateDirectory)"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(SharedObjectLinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)
	@echo rebuilt > $(IntermediateDirectory)/mir_core.relink

MakeIntermediateDirs:
	@$(MakeDirCommand) "$(IntermediateDirectory)"
	@$(MakeDirCommand) "$(OutputDirectory)"

$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "$(IntermediateDirectory)"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/src_logger.cpp$(ObjectSuffix): src/logger.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/logger.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_logger.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_logger.cpp$(PreprocessSuffix): src/logger.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_logger.cpp$(PreprocessSuffix) src/logger.cpp

$(IntermediateDirectory)/src_md5.cpp$(ObjectSuffix): src/md5.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/md5.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_md5.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_md5.cpp$(PreprocessSuffix): src/md5.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_md5.cpp$(PreprocessSuffix) src/md5.cpp

$(IntermediateDirectory)/src_mstring.cpp$(ObjectSuffix): src/mstring.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/mstring.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_mstring.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_mstring.cpp$(PreprocessSuffix): src/mstring.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_mstring.cpp$(PreprocessSuffix) src/mstring.cpp

$(IntermediateDirectory)/src_db.cpp$(ObjectSuffix): src/db.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/db.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_db.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_db.cpp$(PreprocessSuffix): src/db.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_db.cpp$(PreprocessSuffix) src/db.cpp

$(IntermediateDirectory)/src_Linux_CDlgBase.cpp$(ObjectSuffix): src/Linux/CDlgBase.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/Linux/CDlgBase.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Linux_CDlgBase.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Linux_CDlgBase.cpp$(PreprocessSuffix): src/Linux/CDlgBase.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Linux_CDlgBase.cpp$(PreprocessSuffix) src/Linux/CDlgBase.cpp

$(IntermediateDirectory)/src_stdafx.cxx$(ObjectSuffix): src/stdafx.cxx 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/stdafx.cxx" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_stdafx.cxx$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_stdafx.cxx$(PreprocessSuffix): src/stdafx.cxx
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_stdafx.cxx$(PreprocessSuffix) src/stdafx.cxx

$(IntermediateDirectory)/src_http.cpp$(ObjectSuffix): src/http.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/http.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_http.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_http.cpp$(PreprocessSuffix): src/http.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_http.cpp$(PreprocessSuffix) src/http.cpp

$(IntermediateDirectory)/src_tinyxml2.cpp$(ObjectSuffix): src/tinyxml2.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/tinyxml2.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_tinyxml2.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_tinyxml2.cpp$(PreprocessSuffix): src/tinyxml2.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_tinyxml2.cpp$(PreprocessSuffix) src/tinyxml2.cpp

$(IntermediateDirectory)/src_sha256.cpp$(ObjectSuffix): src/sha256.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/sha256.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_sha256.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_sha256.cpp$(PreprocessSuffix): src/sha256.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_sha256.cpp$(PreprocessSuffix) src/sha256.cpp

$(IntermediateDirectory)/src_utf.cpp$(ObjectSuffix): src/utf.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/utf.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_utf.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_utf.cpp$(PreprocessSuffix): src/utf.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_utf.cpp$(PreprocessSuffix) src/utf.cpp

$(IntermediateDirectory)/src_Linux_strutil.cpp$(ObjectSuffix): src/Linux/strutil.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/Linux/strutil.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Linux_strutil.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Linux_strutil.cpp$(PreprocessSuffix): src/Linux/strutil.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Linux_strutil.cpp$(PreprocessSuffix) src/Linux/strutil.cpp

$(IntermediateDirectory)/src_utils.cpp$(ObjectSuffix): src/utils.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_utils.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_utils.cpp$(PreprocessSuffix): src/utils.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_utils.cpp$(PreprocessSuffix) src/utils.cpp

$(IntermediateDirectory)/src_memory.cpp$(ObjectSuffix): src/memory.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/memory.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_memory.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_memory.cpp$(PreprocessSuffix): src/memory.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_memory.cpp$(PreprocessSuffix) src/memory.cpp

$(IntermediateDirectory)/src_Linux_CDbLink.cpp$(ObjectSuffix): src/Linux/CDbLink.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/Linux/CDbLink.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Linux_CDbLink.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Linux_CDbLink.cpp$(PreprocessSuffix): src/Linux/CDbLink.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Linux_CDbLink.cpp$(PreprocessSuffix) src/Linux/CDbLink.cpp

$(IntermediateDirectory)/src_Linux_CCtrlBase.cpp$(ObjectSuffix): src/Linux/CCtrlBase.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/Linux/CCtrlBase.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Linux_CCtrlBase.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Linux_CCtrlBase.cpp$(PreprocessSuffix): src/Linux/CCtrlBase.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Linux_CCtrlBase.cpp$(PreprocessSuffix) src/Linux/CCtrlBase.cpp

$(IntermediateDirectory)/src_tinyxml2_utils.cpp$(ObjectSuffix): src/tinyxml2_utils.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/tinyxml2_utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_tinyxml2_utils.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_tinyxml2_utils.cpp$(PreprocessSuffix): src/tinyxml2_utils.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_tinyxml2_utils.cpp$(PreprocessSuffix) src/tinyxml2_utils.cpp

$(IntermediateDirectory)/src_Linux_fileutil.cpp$(ObjectSuffix): src/Linux/fileutil.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/Linux/fileutil.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Linux_fileutil.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Linux_fileutil.cpp$(PreprocessSuffix): src/Linux/fileutil.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Linux_fileutil.cpp$(PreprocessSuffix) src/Linux/fileutil.cpp

$(IntermediateDirectory)/src_lists.cpp$(ObjectSuffix): src/lists.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/lists.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_lists.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_lists.cpp$(PreprocessSuffix): src/lists.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_lists.cpp$(PreprocessSuffix) src/lists.cpp

$(IntermediateDirectory)/src_bitmaps.cpp$(ObjectSuffix): src/bitmaps.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/bitmaps.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_bitmaps.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_bitmaps.cpp$(PreprocessSuffix): src/bitmaps.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_bitmaps.cpp$(PreprocessSuffix) src/bitmaps.cpp

$(IntermediateDirectory)/src_binbuffer.cpp$(ObjectSuffix): src/binbuffer.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/binbuffer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_binbuffer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_binbuffer.cpp$(PreprocessSuffix): src/binbuffer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_binbuffer.cpp$(PreprocessSuffix) src/binbuffer.cpp

$(IntermediateDirectory)/src_sha1.cpp$(ObjectSuffix): src/sha1.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_core/src/sha1.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_sha1.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_sha1.cpp$(PreprocessSuffix): src/sha1.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_sha1.cpp$(PreprocessSuffix) src/sha1.cpp

##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


