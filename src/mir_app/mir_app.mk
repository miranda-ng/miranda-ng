##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=mir_app
ConfigurationName      :=Debug
WorkspaceConfiguration :=Debug
WorkspacePath          :=/home/ghazan/miranda-ng/codelite
ProjectPath            :=/home/ghazan/miranda-ng/src/mir_app
IntermediateDirectory  :=../../codelite/obj/debug/$(ProjectName)
OutDir                 :=$(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=George Hazan
Date                   :=17/05/22
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
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../../include $(IncludeSwitch)../../plugins/ExternalAPI 
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
Objects0=$(IntermediateDirectory)/src_db_ini.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_auth.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_miranda.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_db_intf.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_db_events.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_database.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_contact.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_CMPluginBase.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_addcontact.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_stdafx.cxx$(ObjectSuffix) \
	



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
	@echo rebuilt > $(IntermediateDirectory)/mir_app.relink

MakeIntermediateDirs:
	@$(MakeDirCommand) "$(IntermediateDirectory)"
	@$(MakeDirCommand) "$(OutputDirectory)"

$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "$(IntermediateDirectory)"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/src_db_ini.cpp$(ObjectSuffix): src/db_ini.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_app/src/db_ini.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_db_ini.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_db_ini.cpp$(PreprocessSuffix): src/db_ini.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_db_ini.cpp$(PreprocessSuffix) src/db_ini.cpp

$(IntermediateDirectory)/src_auth.cpp$(ObjectSuffix): src/auth.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_app/src/auth.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_auth.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_auth.cpp$(PreprocessSuffix): src/auth.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_auth.cpp$(PreprocessSuffix) src/auth.cpp

$(IntermediateDirectory)/src_miranda.cpp$(ObjectSuffix): src/miranda.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_app/src/miranda.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_miranda.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_miranda.cpp$(PreprocessSuffix): src/miranda.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_miranda.cpp$(PreprocessSuffix) src/miranda.cpp

$(IntermediateDirectory)/src_db_intf.cpp$(ObjectSuffix): src/db_intf.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_app/src/db_intf.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_db_intf.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_db_intf.cpp$(PreprocessSuffix): src/db_intf.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_db_intf.cpp$(PreprocessSuffix) src/db_intf.cpp

$(IntermediateDirectory)/src_db_events.cpp$(ObjectSuffix): src/db_events.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_app/src/db_events.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_db_events.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_db_events.cpp$(PreprocessSuffix): src/db_events.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_db_events.cpp$(PreprocessSuffix) src/db_events.cpp

$(IntermediateDirectory)/src_database.cpp$(ObjectSuffix): src/database.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_app/src/database.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_database.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_database.cpp$(PreprocessSuffix): src/database.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_database.cpp$(PreprocessSuffix) src/database.cpp

$(IntermediateDirectory)/src_contact.cpp$(ObjectSuffix): src/contact.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_app/src/contact.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_contact.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_contact.cpp$(PreprocessSuffix): src/contact.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_contact.cpp$(PreprocessSuffix) src/contact.cpp

$(IntermediateDirectory)/src_CMPluginBase.cpp$(ObjectSuffix): src/CMPluginBase.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_app/src/CMPluginBase.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_CMPluginBase.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_CMPluginBase.cpp$(PreprocessSuffix): src/CMPluginBase.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_CMPluginBase.cpp$(PreprocessSuffix) src/CMPluginBase.cpp

$(IntermediateDirectory)/src_addcontact.cpp$(ObjectSuffix): src/addcontact.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_app/src/addcontact.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_addcontact.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_addcontact.cpp$(PreprocessSuffix): src/addcontact.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_addcontact.cpp$(PreprocessSuffix) src/addcontact.cpp

$(IntermediateDirectory)/src_stdafx.cxx$(ObjectSuffix): src/stdafx.cxx 
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ghazan/miranda-ng/src/mir_app/src/stdafx.cxx" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_stdafx.cxx$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_stdafx.cxx$(PreprocessSuffix): src/stdafx.cxx
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_stdafx.cxx$(PreprocessSuffix) src/stdafx.cxx

##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


