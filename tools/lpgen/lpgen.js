//***********************************************************************************//
//* Name:         lpgen.js                                                          *//
//* Language:     JScript                                                           *//
//* Function:     Parse Miranda-NG source for generate translation templates        *//
//* Usage:        Generate translation templates for Miranda-NG plugins and Core    *//
//* Usage:        "cscript /nologo lpgen.js"  to run generation in batches          *//
//* Usage:        "cscript /nologo lpgen.js /log:"yes"" to enable console logging   *//
//* Usage:        "cscript /nologo lpgen.js /path:"path_to_folder"" for one plugin  *//
//* Usage:        "cscript /nologo lpgen.js /crap:"yes"" filtered srings in Crap.txt*//
//* Usage:        Double click to choose folder for one plugin template generation  *//
//* Requirements: for batch mode: create folder ..\..\langpacks\english\Plugins     *//
//* Notes:        ;file path\to\file.ext is a SVN file, source of founded string.   *//
//* Notes:        if double line ;file path\to\file.ext exist, everything from the  *//
//* Notes:        first file was removed as duplicates. You can disable duplicates  *//
//* Notes:        removal by specifying /dupes:"yes"                                *//
//* Author:       BasiL                                                             *//
//***********************************************************************************//

//Init Variables
//Create FileSystemObject FSO
var FSO=WScript.CreateObject("Scripting.FileSystemObject");
//FileSystemObject vars
var ForReading=1;
var TristateUseDefault=-2;
var overwritefile=true;
var unicode=false;
//disabling log by default
var log=false;
//remove dupes by default
var dupes=false;
//stream - our variable for output UTF-8 files with BOM
var stream= new ActiveXObject("ADODB.Stream");
stream.Type = 2; // text mode
stream.Charset = "utf-8";

//Path variables
var scriptpath=FSO.GetParentFolderName(WScript.ScriptFullName);
//crazy way to get path two layers upper "\tools\lpgen\"
var trunk=FSO.GetFolder(FSO.GetParentFolderName(FSO.GetParentFolderName(scriptpath)));
//text string for make path output into templates.
var trunkPath=new String(trunk);
//core path
var core=FSO.BuildPath(trunk,"src");
//protocols path
var protocols=FSO.BuildPath(trunk,"Protocols");
//plugins path
var plugins=FSO.BuildPath(trunk,"Plugins");
//langpack folder "\langpacks\english\" in trunk folder
var langpack_en=FSO.BuildPath(trunk,"langpacks\\english");
//Crap.txt will contain strings, which are removed by filtering engine as a garbage, in case if this string are not garbage :)
var crapfile="Crap.txt"
//crap array
var crap=new Array;
//*********************************************************************************//
//                         Checking command line parameters                       *//
//*********************************************************************************//

// if console param /log: specified, set var log true. To enable log, specify /log:"yes"
if (WScript.Arguments.Named.Item("log")) log=true;

// if console param /dupes:"yes" specified, disable duplicated string removal. 
if (WScript.Arguments.Named.Item("dupes")) dupes=true;

//If script run by double click, open choose folder dialog to choose plugin folder to parse. If Cancel pressed, quit script.
if (WScript.FullName.toLowerCase().charAt(WScript.FullName.length - 11)=="w") {
    //Create Shell app object
    objShellApp=WScript.CreateObject("Shell.Application");
    //Open browse for folder dialog
    objFolder=objShellApp.BrowseForFolder(0, "Choose plugin source files folder\nto generate translation template", 512, 17);
    //process generate translate for chosen folder, else quit.
    if (objFolder) {
        //Call GeneratePluginTranslate for chosen folder, output result to "scriptpath"
        GeneratePluginTranslate(objFolder.Self.Path,scriptpath);
        //if choosing folder canceled, quit immediately
        } else WScript.Quit();
}

//when /plugin: specified, parse only this path and quit
if (WScript.Arguments.Named.Item("path")) {
    //Call GeneratePluginTranslate for path specified in command line argument /path:"path/to/plugin", output result to "scriptpath"
    GeneratePluginTranslate(WScript.Arguments.Named.Item("path"),scriptpath);
    //Write gargage crap array into crap file
    WriteToFile(crap,plugin+"_crap.txt");
    //We are done, quit.
    WScript.Quit();
}

//*********************************************************************************//
//                                    Main part                                   *//
//*********************************************************************************//

//Generate =CORE=.txt
GenerateCore();

//generate plugins, listed in mir_full.sln
//Init array with files
project_files=new Array;
//open mir_full.sln
sln_stream=FSO.GetFile(trunk+"\\bin10\\mir_full.sln").OpenAsTextStream(ForReading, TristateUseDefault);
//Reading line-by-line
 while (!sln_stream.AtEndOfStream) {
     //Init regexp array for getting only $1 from regexp search
     regexpstring=new Array();
     //read on line into rcline
     slnline=sln_stream.ReadLine();
     //find string to translate in rcline by regexp
     regexpstring=slnline.match(/(?:Project\(\"\{.*?,\x20*?\"\.\.)(\\(:?plugins|protocols).*vcxproj)(?=",)/i);      
     // if exist regexpstring, add to array, adding "trunk"
     if (regexpstring) {project_files.push(trunk+regexpstring[1])};
    };
//closing file
sln_stream.Close();
//ok, now we have all project files in array, let's add Pascal files to this array.
//Check protocols folder, than plugins folder. Unfortunatelly, this works badly.
//FindFiles(protocols,"\\.dpr$",project_files);
//FindFiles(plugins,"\\.dpr$",project_files);
//push pascal project files directly
// project_files.push(trunk+"\\plugins\\Actman\\actman.dpr");
// project_files.push(trunk+"\\plugins\\HistoryPlusPlus\\historypp.dpr");
// project_files.push(trunk+"\\plugins\\ImportTXT\\importtxt.dpr");
// project_files.push(trunk+"\\plugins\\QuickSearch\\quicksearch.dpr");
// project_files.push(trunk+"\\plugins\\ShlExt\\shlext.dpr");
// project_files.push(trunk+"\\plugins\\Watrack\\watrack.dpr");
// project_files.push(trunk+"\\plugins\\mRadio\\mradio.dpr");

//create Enumerator with project files from sln and dpr file find results, sorted alphabetically
files=new Enumerator(project_files.sort());
while (!files.atEnd()) {
    //get file name
    file=FSO.GetFile(files.item());
    //get parent folder name
    plugfolder=FSO.GetParentFolderName(file);
    //call function for plugin folder, output to plugins folder.
    GeneratePluginTranslate(plugfolder,langpack_en+"\\Plugins",file);
    //WScript.Echo(file);
    //next project file
    files.moveNext();
};
//Write Crap to file.
if (WScript.Arguments.Named.Item("crap")) WriteToFile(crap,crapfile);

//*********************************************************************************//
//                                    Functions                                   *//
//*********************************************************************************//

//Generate =CORE=.txt
function GenerateCore() {
 //init arrays
 corestrings=new Array();
 core_src=new Array();
 core_rc=new Array();
 //if log parameter specified, output a log.
 if (log) WScript.Echo("Processing CORE...");
 //first string is necessary for Miranda-NG to load langpack
 corestrings.push("Miranda Language Pack Version 1");
 //define core filename. File will be overwritten!
 corefile=FSO.BuildPath(langpack_en,"=CORE=.txt");
 //find all *.rc files and list files in array
 FindFiles(core,"\\.rc$",core_rc);
 //find all source files and list files in array
 FindFiles(core,"\\.h$|\\.cpp$|\\.c$",core_src);
 //Parse files "core_rc", put result into "corestrings" using "ParseRCFile" function
 ParseFiles(core_rc,corestrings,ParseRCFile);
 //Parse files "core_src", put result into "corestrings" using "ParseSourceFile" function
 ParseFiles(core_src,corestrings,ParseSourceFile);
 //Now we have all strings in "corestrings", next we remove duplicate strings from array and put results into "nodupes"
 nodupes=eliminateDuplicates(corestrings);
 //if dupes requred, make nodupes with du
 if (dupes) nodupes=corestrings;
 //logging results
 if (log) WScript.Echo("Writing "+nodupes.length+" strings for CORE");
 //finally, write "nodupes" array to file
 WriteToUnicodeFile(nodupes,corefile);
}

//Make a translation template for plugin in "pluginpath", put generated file into "langpackfilepath"
function GeneratePluginTranslate (pluginpath,langpackfilepath,vcxprojfile) {
    //init arrays with files to parse
    resourcefiles=new Array();
    sourcefiles=new Array();
    versionfile=new Array();
    //init array with muuid+"head"
    head=new Array();
    //init array with strings from parsed files
    foundstrings=new Array();
    //find a name of our plugin
    //if vcxprojfile param given, use it
    if (vcxprojfile) {
        //get plugin name from vcxprojfile
        plugin=GetPluginName(vcxprojfile);
        } else {
        //if vcxprojfile ommited, try to find plugin name from folder files.
        plugin=GetPluginName(pluginpath);
        };
    //if we didn't find plugin name, return.
    if (!plugin) return;
    //if log parameter specified, output a log.
    if (log) WScript.Echo("Processing...  "+plugin);
    //define langpack filename. File will be overwritten!
    langpack=langpackfilepath+"\\"+plugin+".txt";
    //get MUUID of plugin and put into array as a first string.
    GetMUUID(pluginpath,head);

    //put a name of plugin into array as second string
    //foundstrings.push(";langpack template for "+plugin);

    //Get info from version.h
    //First, locate a version.h file
    FindFiles(pluginpath,"^version.h$",versionfile);
    //Parse version.h file
    ParseVersion_h(versionfile,head);
    //find all *.rc files and list files in array
    FindFiles(pluginpath,"\\.rc$",resourcefiles);
    //find all source files and list files in array
    FindFiles(pluginpath,"\\.h$|\\.cpp$|\\.c$|\\.pas$|\\.dpr$|\\.inc$",sourcefiles);
    //Parse files "resourcefiles", put result into "foundstrings" using "ParseRCFile" function
    ParseFiles(resourcefiles,foundstrings,ParseRCFile);
    //Parse files "sourcefiles", put result into "foundstrings" using "ParseSourceFile" function
    ParseFiles(sourcefiles,foundstrings,ParseSourceFile);
    //Parsing all sources done and head are ready (if version.h exist and plugin are not Pascal). If we still have head with 7 strings (version.h parsed OK, gives us 6 stings + 1 first string always exist in head - MUUID) or head have only one string (version.h wasn't found and head have only MUUID) AND didn't find anything in *.RC and source files, so we didn't find any string and generating file is useless, return from function and out log
    if ((head.length==7 || head.length==1) && foundstrings.length==0) {
        if (log) WScript.Echo("!!!Nothing to translate in "+plugin+"!!!");
        return;
        };
    //Suppose that we parse Pascal plugin, thus head have only one string with MUUID, push a plugin name there
    if (head.length==1 & foundstrings.length>0) {
        head.push(";langpack template for "+plugin);
        };
    //We have all strings in "foundstrings", next we remove duplicate strings from array and put results into "nodupes"
    nodupes=eliminateDuplicates(foundstrings);
	 //if dupes requred, make nodupes with du
    if (dupes) nodupes=foundstrings;
    //combine head and translated strings.
    plugintemplate=head.concat(nodupes);
    //logging results
    if (log) WScript.Echo("Writing "+plugintemplate.length+" strings for "+plugin);
    //finally, write "nodupes" array to file
    WriteToFile(plugintemplate,langpack);
};  

//Recourse find all files in "path" with file RegExp mask "name" and return file list into filelistarray
function FindFiles (path,name,filelistarray) {
 //Init vars
 var Folder, Folders, Files, file, filename;
 // second param "name" is our case insensive RegExp
 var filemask=new RegExp(name,"i");
 //Put path into var Folder
 Folder=FSO.GetFolder(path);
 //put subFolders into var
 Folders=new Enumerator(Folder.SubFolders);
 //Create Enumerator with Folder files inside
 Files=new Enumerator(Folder.Files);
 //Cycle by files in Folder
 while (!Files.atEnd()) {
     //file is a next file
     file=Files.item();
     //put file name into filename
     filename=FSO.GetFileName(file);
     //if filename is our name mask, do the job.
     if (filemask.test(filename)) filelistarray.push(file);
     //move to next file
     Files.moveNext();
    };
 //Cycle by subfolders
 while (!Folders.atEnd()) {
    FindFiles(Folders.item().Path,name,filelistarray);
    //WScript.Echo(Folders.item().Path);
    Folders.moveNext();
    };
};

//Find a name for langpack file from source
function GetPluginName (folder_or_file) {
 //check our parameter file or folder?
 if (FSO.FileExists(folder_or_file)) {
     //yes, it's a file, set plugin_project_file as a target
     plugin_project_file=FSO.GetFile(folder_or_file);
     } else {
     //Given parameter is a folder, init file list array
     plugin_project_files=new Array();
     //find project files and put to array
     FindFiles(folder_or_file,"\\.vcxproj$|\\.dpr$",plugin_project_files);
     //if there is nothing found, that's mean this is not a plugin, return from function
     if (!plugin_project_files[0]) return;
     //hope, that project file is first file
     plugin_project_file=plugin_project_files[0]
     }
 //open as text stream
 plugin_project=plugin_project_file.OpenAsTextStream(ForReading, TristateUseDefault);
 //read all file into var project
 project=plugin_project.ReadAll();
 //find a <ProjectName>%langpackfilename%</ProjectName> from *.vcxproj file
 filename=project.match(/<ProjectName>(.+)<\/ProjectName>/);
 if (filename) {
    //return only first item of regexp
    return filename[1];
    } else {
    //There is no specified ProjectName, thus use filename without extension as our langpack name
    return FSO.GetBaseName(plugin_project_file);
    }
}

//Get MUUID of plugin from source file.
function GetMUUID (folder,array) {
 //first, find necessary file list, we are looking for UNICODE_AWARE function, usually this function are in *.cpp, sometimes in *.c and *.h
 //init fillelist array
 muuidfilelist=new Array();
 //search for files in "folder" by mask, put result into muuidfilelist
 FindFiles (folder,"\\.cpp$|\\.c$|\\.h$",muuidfilelist);
 //now we have files, let's put them to Enumerator
 filesenum=new Enumerator(muuidfilelist);
 //cycle through file list and lookup each file for UNICODE_AWARE
 while (!filesenum.atEnd()) {
    //curfile is our current file in files enumerator
    curfile=filesenum.item();
    //If file not zero size, read it and do the job
    if (FSO.GetFile(curfile).Size!=0) {
        //open file as text stream
        file_stream=FSO.GetFile(curfile).OpenAsTextStream(ForReading, TristateUseDefault);
        //this is a regexp to search UNICODE_AWARE
        var find=/(?:UNICODE_AWARE,[\s\S]*?\{)(.+?)(?=\}\s{0,2}\})/g;
        //read file fully into var "allstrings"
        allstrings=file_stream.ReadAll();
        //search regexp in "allstrings" and put results into var "string"
        string=find.exec(allstrings);
        //if current file have found UNICODE_AWARE, var "string" exists, so parse it.
        if (string) {
            //remove spaces, "0x" and "{", in second [1] item of array "string". RegExp "find" have subregexp (.+?), which are our MUUID in "string[1]"
            vals=string[1].replace(/0x|\{|\x20/g,"");
            //now split values of muuid in "vals" by "," into array "values"
            values=vals.split(",");
            //we get array of values, if length of this array not equal to 12 values (starting from zero, thus length is 11), that's mean we found something else, not MUUID. Log it and quit, if length is 12, so check it and generate MUUID          
            if (values.length==11) {
                //now check, is there some values, which have omitted zero after 0x, like in alarms: " 0x4dd7762b, 0xd612, 0x4f84, { 0xaa, 0x86, 0x(no_zero_here_)6, 0x8f, 0x17, 0x85, 0x9b, 0x6d}"
                //first value in values have to be 8 bytes, while length less than 8, add leading "0" to values[0],  
                while (values[0].length<8) {values[0]="0"+values[0]};
                //next two values have to be 4 bytes, adding leading zeroes, while length less than 4.
                for (i=1;i<=2;i++) {
                    while (values[i].length<4) {
                        values[i]="0"+values[i];
                        }
                    }
                //other values have to be 2 bytes, same as above, adding zeroes
                for (i=3;i<=10;i++) {
                    while (values[i].length<2) {
                        values[i]="0"+values[i];
                        }
                    }
                //Push to array founded #muuid
                var muuid="#muuid {"+values[0]+"-"+values[1]+"-"+values[2]+"-"+values[3]+values[4]+"-"+values[5]+values[6]+values[7]+values[8]+values[9]+values[10]+"}";
                };
            };
        //close file
        file_stream.Close();
        }
    //move to next file
    filesenum.moveNext();
    };
 //if we didn't find muuid, put alarm into "muuid"
 if (!muuid) {muuid=";#muuid not found, please specify manually!"}
 //output result into array
 array.push(muuid)
 //log output
 if (log) WScript.Echo(muuid);
};

//Parsing filelist into stringsarray by parsefunction (ParseSourceFile OR ParseRCFile)
function ParseFiles (filelist,stringsarray, parsefunction) {
 //create enumerator filesenum from filelist
 filesenum=new Enumerator(filelist);
 //cycle through file list
 while (!filesenum.atEnd()) {
     //record into current_strings current length of stringsarray
     var current_strings=stringsarray.length;
     var crap_strings=crap.length;
     //curfile is our current file in files enumerator
     curfile=filesenum.item();
     //now apply a parsing function to current file, and put result into stringsarray
     parsefunction(curfile,stringsarray);
     //string variable to cut out a trunkPath from absolute path
     curfilepath=new String(curfile);
     //if after parsing file our stringsarray length greater then var "current_strings", so parsed file return some strings. Thus, we need add a comment with filename
     if (stringsarray.length>current_strings) stringsarray.splice(current_strings,0,";file "+curfilepath.substring(trunkPath.length));
     if (crap.length>crap_strings) crap.splice(crap_strings,0,";file "+curfilepath.substring(trunkPath.length));
     //move to next file
     filesenum.moveNext();
     };
}

//*.RC files line-by-line parser for RC_File, return result into "array"
function ParseRCFile(RC_File,array) {
 //If file zero size, return;
 if (FSO.GetFile(RC_File).Size==0) return;  
 //reading current file
 RC_File_stream=RC_File.OpenAsTextStream(ForReading, TristateUseDefault);
 //Reading line-by-line
 while (!RC_File_stream.AtEndOfStream) {
      //Init regexp array for getting only $1 from regexp search
      regexpstring=new Array();
      //clear up variable
      stringtolangpack="";
      //read on line into rcline
      rcline=RC_File_stream.ReadLine();
      //find string to translate in rcline by regexp
      regexpstring=rcline.match(/\s*(?:CONTROL|(?:DEF)?PUSHBUTTON|[LRC]TEXT|GROUPBOX|CAPTION|MENUITEM|POPUP)\s*\"([^\"]+(\"\")?[^\"]*(\"\")?[^\"]*)\"\,?/);
      
      // if exist regexpstring, do checks, double "" removal and add strings to array
          if (regexpstring) {
          // check for some garbage like "List1","Tab1" etc. in *.rc files, we do not need this.
            switch (regexpstring[1]) {
            case "List1": {break};
            case "List2": {break};
            case "Tab1":  {break};
            case "Tree1": {break};
            case "Tree2": {break};
            case "Spin1": {break};
            case "Spin2": {break};
            case "Spin5": {break};
            case "Custom1": {break};
            case "Custom2": {break};
            case "Slider1": {break};
            case "Slider2": {break};
            //default action is to wrote text inside quoted into array
            default:
            //if there is double "", replace with single one
            stringtolangpack=regexpstring[1].replace(/\"{2}/g,"\"");
            //add string  to array
            array.push("["+stringtolangpack+"]");
            }
        }
    }
 //closing file
 RC_File_stream.Close();
};

//Source files C++ (*.h,*.c,*.cpp) and *.pas,*.dpr,*.inc (Pascal) multiline parser for translations using LPGEN() LPGENT() TranslateT() Translate() _T() TranslateW()
function ParseSourceFile (SourceFile,array) {
 //If file zero size, return;
 if (FSO.GetFile(SourceFile).Size==0) return;
 //open file
 sourcefile_stream=FSO.GetFile(SourceFile).OpenAsTextStream(ForReading, TristateUseDefault);
 //this is final great regexp ever /(?:LPGENT?|Translate[TW]?|_T)(?:[\s])*?(?:\(['"])([\S\s]*?)(?=["']\x20*?\))/mg
 //not store ?: functions LPGEN or LPGENT? or Translate(T or W) or _T, than any unnecessary space \s, than not stored ?: "(" followed by ' or " than \S\s - magic with multiline capture, ending with not stored ?= " or ', than none or few spaces \x20 followed by )/m=multiline g=global
 var find= /(?:LPGENT?|Translate[TW]?|_T)(?:[\s])*?(?:\(['"])([\S\s]*?)(?=["'],?\x20*?(?:tmp)?\))/mg;
  //read file fully into var
 allstrings=sourcefile_stream.ReadAll();
 //now make a job, till end of matching regexp
 while ((string = find.exec(allstrings)) != null) {
    //first, init empty var
    var string;
    //replace newlines with \r\n in second [1] subregexp ([\S\s]*?), Delphi newlines "'#13#10+" also replace 
    onestring=string[1].replace(/'?(\#13\#10)*?\+?\r\n(\x20*?\')?/g,"\\r\\n");
    //remove escape slashes before ' and "
    stringtolangpack=onestring.replace(/\\(['"])/g,"$1");
    //if our string still exist, and length more than 2 symbol (nothing to translate if only two symbols, well, except No and OK, but they are in core. But dozens crap with variables are filtered)
    if (stringtolangpack.length>2) {
        //brand new _T() crap filtering engine :)
        clearstring=filter_T(stringtolangpack);
        //finally put string into array including cover []
        if (clearstring) {array.push("["+clearstring+"]")}
        };
    }
 //close file, we've finish.
 sourcefile_stream.Close();
};

//filter _T() function results
function filter_T(string) {
//filter not begin with symbols :.-]?;#~{!/_+$
var filter1=/^[^\:\-\]\?\;\#\~\|\{\!\/\_\+\\$].+$/g;
//filter string starting from following words
var filter2=/^(SOFTWARE\\|SYSTEM\\|http|ftp|UTF-|utf-|TEXT|EXE|exe|txt|css|html|dat|txt|MS\x20|CLVM|TM_|CLCB|CLSID|CLUI|HKEY_|MButton|BUTTON|WindowClass|MHeader|RichEdit|RICHEDIT|STATIC|EDIT|CList|\d|listbox|LISTBOX|combobox|COMBOBOX|TitleB|std\w|iso-|windows-|<div|<html|<img|<span|<hr|<a\x20|<table|<td|miranda_|kernel32|user32|muc|pubsub|shlwapi|Tahoma|NBRichEdit|CreatePopup|<\/|<\w>|\w\\\w|urn\:|<\?xml|<\!|h\d|\.!\.).*$/g;
//filter string ending with following words
var filter3=/^.+(001|\/value|\*!\*|=)$/g;
//filter from Kildor, different versinos.
//var filter4=/^((%(\d+)?\w\w?)|(\\\w)|(%\w+%)|\.(\w{2,4}|travel|museum|xn--\w+)|\W|\s|\d+)+$/g;
//var filter4=/^((%(\d+)?\w\w?)|(\\\w)|(%\w+%)|(([\w-]+\.)*\.(\w{2,4}|travel|museum|xn--\w+))|\W|\s|\d+)+$/g;
//var filter4=/^((%(\d+)?\w\w?)|(d\s\w)|\[\/?(\w|url|color|)=\]?|(\\\w)|(%\w+%)|(([\w-]+\.)*\.(\w{2,4}|travel|museum|xn--\w+))|\W|\s|\d+)+$/g;
//var filter4=/^((%(\d+)?\w\w?)|(d\s\w)|\[\/?(\w|url|color)(=\w*)?\]?|(\\\w)|(%\w+%)|(([\w-]+\.)*\.(\w{2,4}|travel|museum|xn--\w+))|\W|\s|\d+)+$/g;
//var filter4=/^((%(\d+)?\w\w?)|(d\s\w)|\[\/?(\w|url|img|size|quote|color)(=\w*)?\]?|(\\\w)|(%\w+%)|(([\w-]+\.)*\.(\w{2,4}|travel|museum|xn--\w+))|\W+|\s+|\d+)+$/gi;
//var filter4=/^((%(\d+)?\w\w?)|(d\s\w)|\[\/?(\w|url|img|size|quote|color)(=\w*)?\]?|(\\\w)|(%\w+%)|(([\w-]+\.)*\.(\w{2,4}|travel|museum|xn--\w+))|\W|\s|\d)+$/gi;
var filter4=/^((%(\d+)?\w\w?)|(d\s\w)|\[\/?(\w|url|img|size|quote|color)(=\w*)?\]?|(\\\w)|(%\w+%)|(([\w-]+\.)*\.(\w{2,4}|travel|museum|xn--\w+))|\W|\s|\d)+$/gi;
//filter from Kildor for remove filenames and pathes.
//var filter5=/^[\w-*]+\.\w+$/g;
//var filter5=/^[\w_:%.\\*-]+\.\w+$/g;
var filter5=/^[\w_:%.\\\/*-]+\.\w+$/g;

//apply filters to our string
test1=filter1.test(string);
test2=filter2.test(string);
test3=filter3.test(string);
test4=filter4.test(string);
test5=filter5.test(string);

//if match (test1) first filter and NOT match other tests, thus string are good, return this string back.
if (test1 && !test2 && !test3 && !test4 && !test5) {
    return string;
    } else {
        //in other case, string is a garbage, put into crap array.
        crap.push(string);
        return;
        }
};

//Parse Version.h file to get one translated stirng from "Description" and make a pluging template header.
function ParseVersion_h (VersionFile,array) {
//If file zero size, return;
if (!FSO.GetFileName(VersionFile)) return;
//open file
versionfile_stream=FSO.GetFile(VersionFile).OpenAsTextStream(ForReading, TristateUseDefault);
//read file fully into var
allstrings=versionfile_stream.ReadAll();
//define RegExp for var defines.
var filename=/(?:#define\s+__FILENAME\s+")(.+)(?=")/m;
var pluginname=/(?:#define\s+__PLUGIN_NAME\s+")(.+)(?=")/i;
var author=/(?:#define\s+__AUTHOR\s+")(.+)(?=")/i;
var MAJOR_VERSION=/(?:#define\s+__MAJOR_VERSION\s+)(\d+)/i;
var MINOR_VERSION=/(?:#define\s+__MINOR_VERSION\s+)(\d+)/i;
var RELEASE_NUM=/(?:#define\s+__RELEASE_NUM\s+)(\d+)/i;
var BUILD_NUM=/(?:#define\s+__BUILD_NUM\s+)(\d+)/i;
var description=/(?:#define\s+__DESCRIPTION\s+")(.+)(?=")/i;
//exec RegExps
filename=filename.exec(allstrings);
pluginname=pluginname.exec(allstrings);
MAJOR_VERSION=MAJOR_VERSION.exec(allstrings);
MINOR_VERSION=MINOR_VERSION.exec(allstrings);
RELEASE_NUM=RELEASE_NUM.exec(allstrings);
BUILD_NUM=BUILD_NUM.exec(allstrings);
author=author.exec(allstrings);
description=description.exec(allstrings);

//add a header start mark
array.push(";============================================================");
//push results of regexp vars into array
if (filename) array.push(";  File: "+filename[1]); else array.push(";  File: "+plugin+".dll");
if (pluginname) array.push(";  Plugin: "+pluginname[1]); else array.push(";  Plugin: "+plugin);
if (MAJOR_VERSION) array.push(";  Version: "+MAJOR_VERSION[1]+"."+MINOR_VERSION[1]+"."+RELEASE_NUM[1]+"."+BUILD_NUM[1]); else array.push(";  Version: x.x.x.x");
if (author) array.push(";  Authors: "+fixHexa(author[1])); else array.push(";  Authors: ");
//add a header end mark
array.push(";============================================================");
if (description) array.push("["+description[1]+"]");
//close file
versionfile_stream.Close();
}

//Replaces \x?? hexa codes with their char representation
function fixHexa(string) {
  return string.replace(/\\x([a-fA-F0-9]{2})" "/g,function() {
    return String.fromCharCode(parseInt(arguments[1],16));
  });
}

//Removes duplicates, not mine, found at http://dreaminginjavascript.wordpress.com/2008/08/22/eliminating-duplicates/
function eliminateDuplicates(arr) {
  var i,
      len=arr.length,
      out=[],
      obj={};

  for (i=0;i<len;i++) {
    obj[arr[i]]=0;
  }
  for (i in obj) {
    out.push(i);
  }
  return out;
};

//Put array of strings into file
function WriteToFile (array,langpack) {
 //Create file
 langpackfile=FSO.CreateTextFile(langpack, overwritefile , unicode)
 //Finally, write strings from array to file
 for (i=0;i<=array.length-1;i++) langpackfile.WriteLine(array[i]);
 //Close file
 langpackfile.Close();
};

//Write UTF-8 file
function WriteToUnicodeFile(array,langpack) {
stream.Open();
for (i=0;i<=array.length-1;i++) stream.WriteText(array[i]+"\n");
stream.SaveToFile(langpack, 2);
stream.Close();
}