##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=mir_core
ConfigurationName      :=Release
WorkspaceConfiguration := $(ConfigurationName)
WorkspacePath          :=/home/ghazan/miranda-ng/codelite
ProjectPath            :=/var/www/miranda-ng/src/mir_core
IntermediateDirectory  :=../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core
OutDir                 :=../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=George Hazan
Date                   :=19/12/21
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
OutputFile             :=../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/lib/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
LinkOptions            :=  $(shell wx-config --debug=no --libs --unicode=yes) 
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
CXXFLAGS :=  -O2 $(shell wx-config --cxxflags --debug=no --unicode=yes) -fPIC $(Preprocessors)
CFLAGS   :=  -O2 $(shell wx-config --cxxflags --debug=no --unicode=yes) -fPIC $(Preprocessors)
ASFLAGS  := 
AS       := as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_db.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_http.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_strutil.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_bitmaps.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_fileutil.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2_utils.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utils.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utf.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha256.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_mstring.cpp$(ObjectSuffix) \
	../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha1.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_stdafx.cxx$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_lists.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_memory.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_md5.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_binbuffer.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_logger.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs $(OutputFile)

$(OutputFile): ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/.d $(Objects) 
	@mkdir -p "../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(SharedObjectLinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)
	@echo rebuilt > $(IntermediateDirectory)/mir_core.relink

MakeIntermediateDirs:
	@mkdir -p "../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core"
	@mkdir -p ""../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/lib""

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/.d:
	@mkdir -p "../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core"

PreBuild:


##
## Objects
##
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_db.cpp$(ObjectSuffix): src/db.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_db.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/db.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_db.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_db.cpp$(DependSuffix): src/db.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_db.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_db.cpp$(DependSuffix) -MM src/db.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_db.cpp$(PreprocessSuffix): src/db.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_db.cpp$(PreprocessSuffix) src/db.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_http.cpp$(ObjectSuffix): src/http.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_http.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/http.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_http.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_http.cpp$(DependSuffix): src/http.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_http.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_http.cpp$(DependSuffix) -MM src/http.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_http.cpp$(PreprocessSuffix): src/http.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_http.cpp$(PreprocessSuffix) src/http.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_strutil.cpp$(ObjectSuffix): src/Linux/strutil.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_strutil.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/Linux/strutil.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Linux_strutil.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_strutil.cpp$(DependSuffix): src/Linux/strutil.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_strutil.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_strutil.cpp$(DependSuffix) -MM src/Linux/strutil.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_strutil.cpp$(PreprocessSuffix): src/Linux/strutil.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_strutil.cpp$(PreprocessSuffix) src/Linux/strutil.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_bitmaps.cpp$(ObjectSuffix): src/bitmaps.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_bitmaps.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/bitmaps.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_bitmaps.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_bitmaps.cpp$(DependSuffix): src/bitmaps.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_bitmaps.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_bitmaps.cpp$(DependSuffix) -MM src/bitmaps.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_bitmaps.cpp$(PreprocessSuffix): src/bitmaps.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_bitmaps.cpp$(PreprocessSuffix) src/bitmaps.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_fileutil.cpp$(ObjectSuffix): src/Linux/fileutil.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_fileutil.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/Linux/fileutil.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Linux_fileutil.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_fileutil.cpp$(DependSuffix): src/Linux/fileutil.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_fileutil.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_fileutil.cpp$(DependSuffix) -MM src/Linux/fileutil.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_fileutil.cpp$(PreprocessSuffix): src/Linux/fileutil.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_Linux_fileutil.cpp$(PreprocessSuffix) src/Linux/fileutil.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2_utils.cpp$(ObjectSuffix): src/tinyxml2_utils.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2_utils.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/tinyxml2_utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_tinyxml2_utils.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2_utils.cpp$(DependSuffix): src/tinyxml2_utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2_utils.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2_utils.cpp$(DependSuffix) -MM src/tinyxml2_utils.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2_utils.cpp$(PreprocessSuffix): src/tinyxml2_utils.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2_utils.cpp$(PreprocessSuffix) src/tinyxml2_utils.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utils.cpp$(ObjectSuffix): src/utils.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utils.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_utils.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utils.cpp$(DependSuffix): src/utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utils.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utils.cpp$(DependSuffix) -MM src/utils.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utils.cpp$(PreprocessSuffix): src/utils.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utils.cpp$(PreprocessSuffix) src/utils.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utf.cpp$(ObjectSuffix): src/utf.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utf.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/utf.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_utf.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utf.cpp$(DependSuffix): src/utf.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utf.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utf.cpp$(DependSuffix) -MM src/utf.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utf.cpp$(PreprocessSuffix): src/utf.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_utf.cpp$(PreprocessSuffix) src/utf.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha256.cpp$(ObjectSuffix): src/sha256.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha256.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/sha256.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_sha256.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha256.cpp$(DependSuffix): src/sha256.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha256.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha256.cpp$(DependSuffix) -MM src/sha256.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha256.cpp$(PreprocessSuffix): src/sha256.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha256.cpp$(PreprocessSuffix) src/sha256.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_mstring.cpp$(ObjectSuffix): src/mstring.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_mstring.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/mstring.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_mstring.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_mstring.cpp$(DependSuffix): src/mstring.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_mstring.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_mstring.cpp$(DependSuffix) -MM src/mstring.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_mstring.cpp$(PreprocessSuffix): src/mstring.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_mstring.cpp$(PreprocessSuffix) src/mstring.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha1.cpp$(ObjectSuffix): src/sha1.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha1.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/sha1.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_sha1.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha1.cpp$(DependSuffix): src/sha1.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha1.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha1.cpp$(DependSuffix) -MM src/sha1.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha1.cpp$(PreprocessSuffix): src/sha1.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_sha1.cpp$(PreprocessSuffix) src/sha1.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_stdafx.cxx$(ObjectSuffix): src/stdafx.cxx ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_stdafx.cxx$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/stdafx.cxx" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_stdafx.cxx$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_stdafx.cxx$(DependSuffix): src/stdafx.cxx
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_stdafx.cxx$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_stdafx.cxx$(DependSuffix) -MM src/stdafx.cxx

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_stdafx.cxx$(PreprocessSuffix): src/stdafx.cxx
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_stdafx.cxx$(PreprocessSuffix) src/stdafx.cxx

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_lists.cpp$(ObjectSuffix): src/lists.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_lists.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/lists.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_lists.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_lists.cpp$(DependSuffix): src/lists.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_lists.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_lists.cpp$(DependSuffix) -MM src/lists.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_lists.cpp$(PreprocessSuffix): src/lists.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_lists.cpp$(PreprocessSuffix) src/lists.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2.cpp$(ObjectSuffix): src/tinyxml2.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/tinyxml2.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_tinyxml2.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2.cpp$(DependSuffix): src/tinyxml2.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2.cpp$(DependSuffix) -MM src/tinyxml2.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2.cpp$(PreprocessSuffix): src/tinyxml2.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_tinyxml2.cpp$(PreprocessSuffix) src/tinyxml2.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_memory.cpp$(ObjectSuffix): src/memory.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_memory.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/memory.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_memory.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_memory.cpp$(DependSuffix): src/memory.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_memory.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_memory.cpp$(DependSuffix) -MM src/memory.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_memory.cpp$(PreprocessSuffix): src/memory.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_memory.cpp$(PreprocessSuffix) src/memory.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_md5.cpp$(ObjectSuffix): src/md5.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_md5.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/md5.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_md5.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_md5.cpp$(DependSuffix): src/md5.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_md5.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_md5.cpp$(DependSuffix) -MM src/md5.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_md5.cpp$(PreprocessSuffix): src/md5.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_md5.cpp$(PreprocessSuffix) src/md5.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_binbuffer.cpp$(ObjectSuffix): src/binbuffer.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_binbuffer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/binbuffer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_binbuffer.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_binbuffer.cpp$(DependSuffix): src/binbuffer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_binbuffer.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_binbuffer.cpp$(DependSuffix) -MM src/binbuffer.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_binbuffer.cpp$(PreprocessSuffix): src/binbuffer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_binbuffer.cpp$(PreprocessSuffix) src/binbuffer.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_logger.cpp$(ObjectSuffix): src/logger.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_logger.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_core/src/logger.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_logger.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_logger.cpp$(DependSuffix): src/logger.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_logger.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_logger.cpp$(DependSuffix) -MM src/logger.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_logger.cpp$(PreprocessSuffix): src/logger.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core/src_logger.cpp$(PreprocessSuffix) src/logger.cpp


-include ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_core//*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


