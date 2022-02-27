##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=mir_app
ConfigurationName      :=Debug
WorkspaceConfiguration := $(ConfigurationName)
WorkspacePath          :=/home/ghazan/miranda-ng/codelite
ProjectPath            :=/var/www/miranda-ng/src/mir_app
IntermediateDirectory  :=../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app
OutDir                 :=../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=George Hazan
Date                   :=27/02/22
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
Objects0=../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_CMPluginBase.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_database.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_stdafx.cxx$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contact.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_events.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_ini.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_intf.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_addcontact.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_util.cpp$(ObjectSuffix) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contacts.cpp$(ObjectSuffix) \
	



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs $(OutputFile)

$(OutputFile): ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/.d $(Objects) 
	@mkdir -p "../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(SharedObjectLinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)
	@echo rebuilt > $(IntermediateDirectory)/mir_app.relink

MakeIntermediateDirs:
	@mkdir -p "../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app"
	@mkdir -p ""../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/lib""

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/.d:
	@mkdir -p "../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app"

PreBuild:


##
## Objects
##
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_CMPluginBase.cpp$(ObjectSuffix): src/CMPluginBase.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_CMPluginBase.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/CMPluginBase.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_CMPluginBase.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_CMPluginBase.cpp$(DependSuffix): src/CMPluginBase.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_CMPluginBase.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_CMPluginBase.cpp$(DependSuffix) -MM src/CMPluginBase.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_CMPluginBase.cpp$(PreprocessSuffix): src/CMPluginBase.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_CMPluginBase.cpp$(PreprocessSuffix) src/CMPluginBase.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_database.cpp$(ObjectSuffix): src/database.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_database.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/database.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_database.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_database.cpp$(DependSuffix): src/database.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_database.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_database.cpp$(DependSuffix) -MM src/database.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_database.cpp$(PreprocessSuffix): src/database.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_database.cpp$(PreprocessSuffix) src/database.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_stdafx.cxx$(ObjectSuffix): src/stdafx.cxx ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_stdafx.cxx$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/stdafx.cxx" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_stdafx.cxx$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_stdafx.cxx$(DependSuffix): src/stdafx.cxx
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_stdafx.cxx$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_stdafx.cxx$(DependSuffix) -MM src/stdafx.cxx

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_stdafx.cxx$(PreprocessSuffix): src/stdafx.cxx
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_stdafx.cxx$(PreprocessSuffix) src/stdafx.cxx

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contact.cpp$(ObjectSuffix): src/contact.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contact.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/contact.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_contact.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contact.cpp$(DependSuffix): src/contact.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contact.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contact.cpp$(DependSuffix) -MM src/contact.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contact.cpp$(PreprocessSuffix): src/contact.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contact.cpp$(PreprocessSuffix) src/contact.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_events.cpp$(ObjectSuffix): src/db_events.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_events.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/db_events.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_db_events.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_events.cpp$(DependSuffix): src/db_events.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_events.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_events.cpp$(DependSuffix) -MM src/db_events.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_events.cpp$(PreprocessSuffix): src/db_events.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_events.cpp$(PreprocessSuffix) src/db_events.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_ini.cpp$(ObjectSuffix): src/db_ini.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_ini.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/db_ini.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_db_ini.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_ini.cpp$(DependSuffix): src/db_ini.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_ini.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_ini.cpp$(DependSuffix) -MM src/db_ini.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_ini.cpp$(PreprocessSuffix): src/db_ini.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_ini.cpp$(PreprocessSuffix) src/db_ini.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_intf.cpp$(ObjectSuffix): src/db_intf.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_intf.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/db_intf.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_db_intf.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_intf.cpp$(DependSuffix): src/db_intf.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_intf.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_intf.cpp$(DependSuffix) -MM src/db_intf.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_intf.cpp$(PreprocessSuffix): src/db_intf.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_intf.cpp$(PreprocessSuffix) src/db_intf.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_addcontact.cpp$(ObjectSuffix): src/addcontact.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_addcontact.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/addcontact.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_addcontact.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_addcontact.cpp$(DependSuffix): src/addcontact.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_addcontact.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_addcontact.cpp$(DependSuffix) -MM src/addcontact.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_addcontact.cpp$(PreprocessSuffix): src/addcontact.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_addcontact.cpp$(PreprocessSuffix) src/addcontact.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_util.cpp$(ObjectSuffix): src/db_util.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_util.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/db_util.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_db_util.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_util.cpp$(DependSuffix): src/db_util.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_util.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_util.cpp$(DependSuffix) -MM src/db_util.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_util.cpp$(PreprocessSuffix): src/db_util.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_db_util.cpp$(PreprocessSuffix) src/db_util.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contacts.cpp$(ObjectSuffix): src/contacts.cpp ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contacts.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/var/www/miranda-ng/src/mir_app/src/contacts.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_contacts.cpp$(ObjectSuffix) $(IncludePath)
../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contacts.cpp$(DependSuffix): src/contacts.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contacts.cpp$(ObjectSuffix) -MF../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contacts.cpp$(DependSuffix) -MM src/contacts.cpp

../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contacts.cpp$(PreprocessSuffix): src/contacts.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app/src_contacts.cpp$(PreprocessSuffix) src/contacts.cpp


-include ../../../../../home/ghazan/miranda-ng/codelite/build-$(ConfigurationName)/__/__/__/__/var/www/miranda-ng/src/mir_app//*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


