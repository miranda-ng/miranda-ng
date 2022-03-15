##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=mir_app
ConfigurationName      :=Debug
WorkspaceConfiguration := $(ConfigurationName)
WorkspacePath          :=/var/www/miranda-ng/codelite
ProjectPath            :=/var/www/miranda-ng/src/mir_app
IntermediateDirectory  :=../../codelite/build-$(ConfigurationName)/__/src/mir_app
OutDir                 :=../../codelite/build-$(ConfigurationName)/__/src/mir_app
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
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../../include $(IncludeSwitch)../../plugins/ExternalAPI 
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
Objects0=../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_intf.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_events.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_addcontact.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_contact.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_stdafx.cxx$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_database.cpp$(ObjectSuffix) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_CMPluginBase.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs $(OutputFile)

$(OutputFile): ../../codelite/build-$(ConfigurationName)/__/src/mir_app/.d $(Objects) 
	@mkdir -p "../../codelite/build-$(ConfigurationName)/__/src/mir_app"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(SharedObjectLinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)
	@echo rebuilt > $(IntermediateDirectory)/mir_app.relink

MakeIntermediateDirs:
	@mkdir -p "../../codelite/build-$(ConfigurationName)/__/src/mir_app"
	@mkdir -p ""../../codelite/build-$(ConfigurationName)/lib""

../../codelite/build-$(ConfigurationName)/__/src/mir_app/.d:
	@mkdir -p "../../codelite/build-$(ConfigurationName)/__/src/mir_app"

PreBuild:


##
## Objects
##
../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_intf.cpp$(ObjectSuffix): src/db_intf.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_intf.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/db_intf.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_db_intf.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_intf.cpp$(DependSuffix): src/db_intf.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_intf.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_intf.cpp$(DependSuffix) -MM src/db_intf.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_intf.cpp$(PreprocessSuffix): src/db_intf.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_intf.cpp$(PreprocessSuffix) src/db_intf.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_events.cpp$(ObjectSuffix): src/db_events.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_events.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/db_events.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_db_events.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_events.cpp$(DependSuffix): src/db_events.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_events.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_events.cpp$(DependSuffix) -MM src/db_events.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_events.cpp$(PreprocessSuffix): src/db_events.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_db_events.cpp$(PreprocessSuffix) src/db_events.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_addcontact.cpp$(ObjectSuffix): src/addcontact.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_addcontact.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/addcontact.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_addcontact.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_addcontact.cpp$(DependSuffix): src/addcontact.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_addcontact.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_addcontact.cpp$(DependSuffix) -MM src/addcontact.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_addcontact.cpp$(PreprocessSuffix): src/addcontact.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_addcontact.cpp$(PreprocessSuffix) src/addcontact.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_contact.cpp$(ObjectSuffix): src/contact.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_contact.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/contact.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_contact.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_contact.cpp$(DependSuffix): src/contact.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_contact.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_contact.cpp$(DependSuffix) -MM src/contact.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_contact.cpp$(PreprocessSuffix): src/contact.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_contact.cpp$(PreprocessSuffix) src/contact.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_stdafx.cxx$(ObjectSuffix): src/stdafx.cxx ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_stdafx.cxx$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/stdafx.cxx" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_stdafx.cxx$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_stdafx.cxx$(DependSuffix): src/stdafx.cxx
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_stdafx.cxx$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_stdafx.cxx$(DependSuffix) -MM src/stdafx.cxx

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_stdafx.cxx$(PreprocessSuffix): src/stdafx.cxx
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_stdafx.cxx$(PreprocessSuffix) src/stdafx.cxx

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_database.cpp$(ObjectSuffix): src/database.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_database.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/database.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_database.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_database.cpp$(DependSuffix): src/database.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_database.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_database.cpp$(DependSuffix) -MM src/database.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_database.cpp$(PreprocessSuffix): src/database.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_database.cpp$(PreprocessSuffix) src/database.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_CMPluginBase.cpp$(ObjectSuffix): src/CMPluginBase.cpp ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_CMPluginBase.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/CMPluginBase.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_CMPluginBase.cpp$(ObjectSuffix) $(IncludePath)
../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_CMPluginBase.cpp$(DependSuffix): src/CMPluginBase.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_CMPluginBase.cpp$(ObjectSuffix) -MF../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_CMPluginBase.cpp$(DependSuffix) -MM src/CMPluginBase.cpp

../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_CMPluginBase.cpp$(PreprocessSuffix): src/CMPluginBase.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../codelite/build-$(ConfigurationName)/__/src/mir_app/src_CMPluginBase.cpp$(PreprocessSuffix) src/CMPluginBase.cpp


-include ../../codelite/build-$(ConfigurationName)/__/src/mir_app//*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


