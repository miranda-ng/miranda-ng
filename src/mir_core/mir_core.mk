##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=mir_core
ConfigurationName      :=Debug
WorkspaceConfiguration := $(ConfigurationName)
WorkspacePath          :=/var/www/miranda-ng/codelite
ProjectPath            :=/var/www/miranda-ng/src/mir_core
IntermediateDirectory  :=../../codelite/build-$(ConfigurationName)/__/src/mir_core
OutDir                 :=../../codelite/build-$(ConfigurationName)/__/src/mir_core
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=George Hazan
Date                   :=15/03/22
CodeLitePath           :=/home/ghazan/.codelite
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
OutputFile             :=../../codelite/build-$(ConfigurationName)/lib/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
LinkOptions            :=  $(shell wx-config   --libs --unicode=yes)
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../../include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
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
Objects0=../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_db.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_http.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_strutil.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_bitmaps.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_fileutil.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2_utils.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utils.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utf.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha256.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_mstring.cpp$(ObjectSuffix) \
	../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha1.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_stdafx.cxx$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_lists.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_memory.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_md5.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_binbuffer.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_logger.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs $(OutputFile)

$(OutputFile): ../../codelite/build-$(ConfigurationName)/__/src/mir_core/.d $(Objects) 
	@mkdir -p "../../codelite/build-$(ConfigurationName)/__/src/mir_core"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(SharedObjectLinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)
	@echo rebuilt > $(IntermediateDirectory)/mir_core.relink

MakeIntermediateDirs:
	@mkdir -p "../../codelite/build-$(ConfigurationName)/__/src/mir_core"
	@mkdir -p ""../../codelite/build-$(ConfigurationName)/lib""

../../codelite/build-$(ConfigurationName)/__/src/mir_core/.d:
	@mkdir -p "../../codelite/build-$(ConfigurationName)/__/src/mir_core"

PreBuild:


##
## Objects
##
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_db.cpp$(ObjectSuffix): src/db.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_db.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/db.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_db.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_db.cpp$(DependSuffix): src/db.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_db.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_db.cpp$(DependSuffix) -MM src/db.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_db.cpp$(PreprocessSuffix): src/db.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_db.cpp$(PreprocessSuffix) src/db.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_http.cpp$(ObjectSuffix): src/http.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_http.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/http.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_http.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_http.cpp$(DependSuffix): src/http.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_http.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_http.cpp$(DependSuffix) -MM src/http.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_http.cpp$(PreprocessSuffix): src/http.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_http.cpp$(PreprocessSuffix) src/http.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_strutil.cpp$(ObjectSuffix): src/Linux/strutil.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_strutil.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/Linux/strutil.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Linux_strutil.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_strutil.cpp$(DependSuffix): src/Linux/strutil.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_strutil.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_strutil.cpp$(DependSuffix) -MM src/Linux/strutil.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_strutil.cpp$(PreprocessSuffix): src/Linux/strutil.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_strutil.cpp$(PreprocessSuffix) src/Linux/strutil.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_bitmaps.cpp$(ObjectSuffix): src/bitmaps.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_bitmaps.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/bitmaps.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_bitmaps.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_bitmaps.cpp$(DependSuffix): src/bitmaps.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_bitmaps.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_bitmaps.cpp$(DependSuffix) -MM src/bitmaps.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_bitmaps.cpp$(PreprocessSuffix): src/bitmaps.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_bitmaps.cpp$(PreprocessSuffix) src/bitmaps.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_fileutil.cpp$(ObjectSuffix): src/Linux/fileutil.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_fileutil.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/Linux/fileutil.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Linux_fileutil.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_fileutil.cpp$(DependSuffix): src/Linux/fileutil.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_fileutil.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_fileutil.cpp$(DependSuffix) -MM src/Linux/fileutil.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_fileutil.cpp$(PreprocessSuffix): src/Linux/fileutil.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_Linux_fileutil.cpp$(PreprocessSuffix) src/Linux/fileutil.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2_utils.cpp$(ObjectSuffix): src/tinyxml2_utils.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2_utils.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/tinyxml2_utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_tinyxml2_utils.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2_utils.cpp$(DependSuffix): src/tinyxml2_utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2_utils.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2_utils.cpp$(DependSuffix) -MM src/tinyxml2_utils.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2_utils.cpp$(PreprocessSuffix): src/tinyxml2_utils.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2_utils.cpp$(PreprocessSuffix) src/tinyxml2_utils.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utils.cpp$(ObjectSuffix): src/utils.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utils.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_utils.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utils.cpp$(DependSuffix): src/utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utils.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utils.cpp$(DependSuffix) -MM src/utils.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utils.cpp$(PreprocessSuffix): src/utils.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utils.cpp$(PreprocessSuffix) src/utils.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utf.cpp$(ObjectSuffix): src/utf.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utf.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/utf.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_utf.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utf.cpp$(DependSuffix): src/utf.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utf.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utf.cpp$(DependSuffix) -MM src/utf.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utf.cpp$(PreprocessSuffix): src/utf.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_utf.cpp$(PreprocessSuffix) src/utf.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha256.cpp$(ObjectSuffix): src/sha256.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha256.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/sha256.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_sha256.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha256.cpp$(DependSuffix): src/sha256.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha256.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha256.cpp$(DependSuffix) -MM src/sha256.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha256.cpp$(PreprocessSuffix): src/sha256.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha256.cpp$(PreprocessSuffix) src/sha256.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_mstring.cpp$(ObjectSuffix): src/mstring.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_mstring.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/mstring.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_mstring.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_mstring.cpp$(DependSuffix): src/mstring.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_mstring.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_mstring.cpp$(DependSuffix) -MM src/mstring.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_mstring.cpp$(PreprocessSuffix): src/mstring.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_mstring.cpp$(PreprocessSuffix) src/mstring.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha1.cpp$(ObjectSuffix): src/sha1.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha1.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/sha1.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_sha1.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha1.cpp$(DependSuffix): src/sha1.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha1.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha1.cpp$(DependSuffix) -MM src/sha1.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha1.cpp$(PreprocessSuffix): src/sha1.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_sha1.cpp$(PreprocessSuffix) src/sha1.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_stdafx.cxx$(ObjectSuffix): src/stdafx.cxx ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_stdafx.cxx$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/stdafx.cxx" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_stdafx.cxx$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_stdafx.cxx$(DependSuffix): src/stdafx.cxx
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_stdafx.cxx$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_stdafx.cxx$(DependSuffix) -MM src/stdafx.cxx

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_stdafx.cxx$(PreprocessSuffix): src/stdafx.cxx
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_stdafx.cxx$(PreprocessSuffix) src/stdafx.cxx

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_lists.cpp$(ObjectSuffix): src/lists.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_lists.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/lists.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_lists.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_lists.cpp$(DependSuffix): src/lists.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_lists.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_lists.cpp$(DependSuffix) -MM src/lists.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_lists.cpp$(PreprocessSuffix): src/lists.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_lists.cpp$(PreprocessSuffix) src/lists.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2.cpp$(ObjectSuffix): src/tinyxml2.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/tinyxml2.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_tinyxml2.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2.cpp$(DependSuffix): src/tinyxml2.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2.cpp$(DependSuffix) -MM src/tinyxml2.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2.cpp$(PreprocessSuffix): src/tinyxml2.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_tinyxml2.cpp$(PreprocessSuffix) src/tinyxml2.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_memory.cpp$(ObjectSuffix): src/memory.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_memory.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/memory.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_memory.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_memory.cpp$(DependSuffix): src/memory.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_memory.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_memory.cpp$(DependSuffix) -MM src/memory.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_memory.cpp$(PreprocessSuffix): src/memory.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_memory.cpp$(PreprocessSuffix) src/memory.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_md5.cpp$(ObjectSuffix): src/md5.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_md5.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/md5.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_md5.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_md5.cpp$(DependSuffix): src/md5.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_md5.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_md5.cpp$(DependSuffix) -MM src/md5.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_md5.cpp$(PreprocessSuffix): src/md5.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_md5.cpp$(PreprocessSuffix) src/md5.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_binbuffer.cpp$(ObjectSuffix): src/binbuffer.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_binbuffer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/binbuffer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_binbuffer.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_binbuffer.cpp$(DependSuffix): src/binbuffer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_binbuffer.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_binbuffer.cpp$(DependSuffix) -MM src/binbuffer.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_binbuffer.cpp$(PreprocessSuffix): src/binbuffer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_binbuffer.cpp$(PreprocessSuffix) src/binbuffer.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_logger.cpp$(ObjectSuffix): src/logger.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_logger.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/logger.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_logger.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_logger.cpp$(DependSuffix): src/logger.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_logger.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_logger.cpp$(DependSuffix) -MM src/logger.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_logger.cpp$(PreprocessSuffix): src/logger.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_core/src_logger.cpp$(PreprocessSuffix) src/logger.cpp


-include ../../codelite/build-$(ConfigurationName)/__/src/mir_core//*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


