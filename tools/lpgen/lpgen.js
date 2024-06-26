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
var FSO = WScript.CreateObject("Scripting.FileSystemObject");
//FileSystemObject vars
var ForReading = 1;
var TristateUseDefault = 0;
var overwritefile = true;
var unicode = false;
//disabling log by default
var log = false;
//remove dupes by default
var dupes = false;
//stream - our variable for output UTF-8 files with BOM
var stream = new ActiveXObject("ADODB.Stream");
//stream var tune
stream.Type = 2; // text mode
stream.Charset = "utf-8";
//plugin - variable for plugin name
var plugin = "";

//Path variables
//lpgen.js script path
var scriptpath = FSO.GetParentFolderName(WScript.ScriptFullName);
//get path two layers upper "\tools\lpgen\"
//string for make path output into templates.
var trunkPath = FSO.GetParentFolderName(FSO.GetParentFolderName(scriptpath));
//path to sln file
var slnfile = FSO.BuildPath(trunkPath, "bin15\\mir_full.sln");
//core path
var core = FSO.BuildPath(trunkPath, "src");
//include path
var include = FSO.BuildPath(trunkPath, "include");
//langpack folder "\langpacks\english\" in trunk folder
var langpack_en = FSO.BuildPath(trunkPath, "langpacks\\english");
//Crap.txt will contain strings, which are removed by filtering engine as a garbage, in case if this string are not garbage :)
var crapfile = "Crap.txt";
//Crap array
var crap = [];

//*********************************************************************************//
//                         Checking command line parameters                       *//
//*********************************************************************************//

// if console param /log: specified, set var log true. To enable log, specify /log:"yes"
if (WScript.Arguments.Named.Item("log")) {
    log = true;
}

// if console param /dupes:"yes" specified, disable duplicated string removal. 
if (WScript.Arguments.Named.Item("dupes")) {
    dupes = true;
}

//If script run by double click, open choose folder dialog to choose plugin folder to parse. If Cancel pressed, quit script.
if (WScript.FullName.toLowerCase().charAt(WScript.FullName.length - 11) == "w") {
    //Create Shell app object
    var objShellApp = WScript.CreateObject("Shell.Application");
    //Open browse for folder dialog
    var objFolder = objShellApp.BrowseForFolder(0, "Choose plugin source files folder\nto generate translation template", 512, 17);
    //process generate translate for chosen folder, else quit.
    if (objFolder) {
        //Call GeneratePluginTranslate for chosen folder, output result to "scriptpath"
        GeneratePluginTranslate(objFolder.Self.Path, scriptpath);
        //if choosing folder canceled, quit immediately
    } else {
        WScript.Quit();
    }
}

//when /plugin: specified, parse only this path and quit
if (WScript.Arguments.Named.Item("path")) {
    //Call GeneratePluginTranslate for path specified in command line argument /path:"path/to/plugin", output result to "scriptpath"
    GeneratePluginTranslate(WScript.Arguments.Named.Item("path"), scriptpath);
    //Write garbage crap array into crap file, if crap exist
    if (crap.length > 0) {
        WriteToFile(crap, plugin + "_crap.txt");
    }
    //We are done, quit.
    WScript.Quit();
}

//*********************************************************************************//
//                                    Main part                                   *//
//*********************************************************************************//

//Generate =CORE=.txt
GenerateCore();

//Generate plugins\protocols, listed in mir_full.sln

//Init array with files path
var project_files = [];
//open mir_full.sln
var sln_stream = FSO.GetFile(slnfile).OpenAsTextStream(ForReading, TristateUseDefault);
//Reading line-by-line
while (!sln_stream.AtEndOfStream) {
    //Init regexp array for our sln parse logic
    var sln_project_regexp = [];
    //read one line into slnline
    var slnline = sln_stream.ReadLine();
    //find a project definition in sln file by RegExp
    sln_project_regexp = slnline.match(/(?:Project\(\"\{[\w\d-]+\}\"\)\x20+\=\x20+\"(.+?)\",\x20*?\"\.\.)(\\(:?plugins|protocols).*vcxproj)(?=",)/i);
    // if exist sln_project_regexp, add to array, adding leading path to "trunk"
    if (sln_project_regexp) {
        //RegExp for unneeded modules, such as crypting library, zlib.dll etc.
        var unneeded_modules = /(Cryptlib)/i;
        // Now check for unneeded modules NOT passed (module name are in sln_project_regexp[1]
        if (!unneeded_modules.test(sln_project_regexp[1])) {
            //no, this is not unneeded module, put path to array. Trunk path + path to file in sln_project_regexp[2]
            project_files.push(trunkPath + sln_project_regexp[2]);
        }
    }
}
//closing file
sln_stream.Close();
//ok, now we have all project files in array, let's add Pascal files to this array directly.
// remove following lines comments to add Pascal plugins processing.
// project_files.push(trunkPath+"\\plugins\\Actman\\actman.dpr");
// project_files.push(trunkPath+"\\plugins\\HistoryPlusPlus\\historypp.dpr");
// project_files.push(trunkPath+"\\plugins\\Watrack\\watrack.dpr");
// project_files.push(trunkPath+"\\plugins\\mRadio\\mradio.dpr");

//create Enumerator with project files from sln and dpr files, sorted alphabetically
var files = new Enumerator(project_files.sort());
while (!files.atEnd()) {
    //get file name
    var file = FSO.GetFile(files.item());
    //get parent folder name
    var plugfolder = FSO.GetParentFolderName(file);
    //call function for plugin folder, output to plugins folder.
    GeneratePluginTranslate(plugfolder, langpack_en + "\\Plugins", file);
    //next project file
    files.moveNext();
}
//Write Crap to file.
if (WScript.Arguments.Named.Item("crap")) {
    WriteToFile(crap, crapfile);
}
//Finished
if (log) {
    WScript.Echo("Finish getting strings from source files.");
}

//*********************************************************************************//
//                                    Functions                                   *//
//*********************************************************************************//

//Generate =CORE=.txt
function GenerateCore() {
    var corefile = "",
        ver = "";
    //init arrays
    var corestrings = [],
        corehead = [],
        core_src = [],
        include_src = [],
        core_rc = [],
        nodupes;
    //if log parameter specified, output a log.
    if (log) {
        WScript.Echo("Processing CORE...");
    }
    //get current core version from build file and replace spaces with dots
    ver = ReadFile(trunkPath + "\\build\\build.no").replace(new RegExp(/\s/g), "\.");
    if (log) {
        WScript.Echo("CORE version is " + ver);
    }
    //first string is necessary for Miranda-NG to load langpack
    //corestrings.push("Miranda Language Pack Version 1"); // TODO: this need to be placed into =HEAD=.txt file or similar
    //add header to =CORE=.txt
    corehead.push(";============================================================");
    corehead.push(";  File: miranda32/64.exe");
    corehead.push(";  Module: Miranda Core");
    corehead.push(";  Version: " + ver);
    corehead.push(";============================================================");
    //find all *.rc files and list files in array
    FindFiles(core, "\\.rc$", core_rc);
    //find all source files and list files in array
    FindFiles(core, "\\.h$|\\.cpp$|\\.c$", core_src);
	//find all source files and list files in array
    FindFiles(include, "\\.h$", include_src);
    //Parse files "core_rc", put result into "corestrings" using "ParseRCFile" function
    ParseFiles(core_rc, corestrings, ParseRCFile);
    //Parse files "core_src", put result into "corestrings" using "ParseSourceFile" function
    ParseFiles(core_src, corestrings, ParseSourceFile);
	//Parse files "include_src", put result into "corestrings" using "ParseSourceFile" function
    ParseFiles(include_src, corestrings, ParseSourceFile);
    //Now we have all strings in "corestrings", next we remove duplicate strings from array and put results into "nodupes"
    nodupes = eliminateDuplicates(corestrings);
    //if dupes required, make nodupes with dupes :)
    if (dupes) {
        nodupes = corestrings;
    }
    //logging results
    if (log) {
        WScript.Echo("Writing " + nodupes.length + " strings for CORE");
    }
    //concatenate head and nodupes
    corestrings = corehead.concat(nodupes);
    //define core filename. File will be overwritten!
    corefile = FSO.BuildPath(langpack_en, "=CORE=.txt");
    //finally, write "nodupes" array to file
    WriteToUnicodeFileNoBOM(corestrings, corefile);
}

//Make a translation template for plugin in "pluginpath", put generated file into "langpackfilepath"
function GeneratePluginTranslate(pluginpath, langpackfilepath, vcxprojfile) {
    var langpack = "",
        nodupes = [],
        plugintemplate =[];
    //init arrays with files to parse
    var resourcefiles = [],
        sourcefiles = [],
        versionfile = [];
    //init array with muuid+"head"
    var head = [];
    //init array with strings from parsed files
    var foundstrings = [];
    //find a name of our plugin
    //if vcxprojfile param given, use it
    if (vcxprojfile) {
        //get plugin name from vcxprojfile
        plugin = GetPluginName(vcxprojfile);
    } else {
        //if vcxprojfile omitted, try to find plugin name from folder files.
        plugin = GetPluginName(pluginpath);
    }
    //if we didn't find plugin name, return.
    if (!plugin) return;
    //if log parameter specified, output a log.
    if (log) {
        WScript.Echo("Processing...  " + plugin);
    }
    //define langpack filename. File will be overwritten!
    langpack = langpackfilepath + "\\" + plugin + ".txt";
    //get MUUID of plugin and put into array as a first string.
    GetMUUID(pluginpath, head);
    //Parse version.h file, put results into array "head"
    ParseVersion_h(pluginpath, head);
    //find all *.rc files and list files in array
    FindFiles(pluginpath, "\\.rc$", resourcefiles);
    //find all source files and list files in array
    FindFiles(pluginpath, "\\.h$|\\.cpp$|\\.c$|\\.pas$|\\.dpr$|\\.inc$", sourcefiles);
    //Parse files "resourcefiles", put result into "foundstrings" using "ParseRCFile" function
    ParseFiles(resourcefiles, foundstrings, ParseRCFile);
    //Parse files "sourcefiles", put result into "foundstrings" using "ParseSourceFile" function
    ParseFiles(sourcefiles, foundstrings, ParseSourceFile);
    //Parsing all sources done and head are ready (if version.h exist and plugin are not Pascal). If we still have head with 7 strings:
    //(version.h parsed OK, gives us 6 stings + 1 first string always exist in head - MUUID)
    //OR head have only one string (version.h wasn't found and head have only MUUID)
    //AND didn't find anything in *.RC and source files, so:
    //we didn't find any string and generating file is useless, return from function and out log
    if ((head.length == 7 || head.length == 1) && foundstrings.length === 0) {
        if (log) {
            WScript.Echo("!!!Nothing to translate in " + plugin + "!!!");
        }
        return;
    }
    //Suppose that we parse Pascal plugin, thus head have only one string with MUUID, push a plugin name there
    if (head.length == 1 && foundstrings.length > 0) {
        head.push(";langpack template for " + plugin);
    }
    //We have all strings in "foundstrings", next we remove duplicate strings from array and put results into "nodupes"
    nodupes = eliminateDuplicates(foundstrings);
    //if dupes required, make nodupes with dupes :)
    if (dupes) {
        nodupes = foundstrings;
    }
    //combine head and translated strings.
    plugintemplate = head.concat(nodupes);
    //logging results
    if (log) {
        WScript.Echo("Writing " + plugintemplate.length + " strings for " + plugin);
    }
    //finally, write "nodupes" array to file
    WriteToUnicodeFileNoBOM(plugintemplate, langpack);
}

//Recourse find all files in "path" with file RegExp mask "name" and return file list into filelistarray
function FindFiles(path, name, filelistarray) {
    //Init vars
    var Folder, Folders, Files, file, filename;
    // second param "name" is our case insensitive RegExp
    var filemask = new RegExp(name, "i");
    //Put path into var Folder
    Folder = FSO.GetFolder(path);
    //put subFolders into var
    Folders = new Enumerator(Folder.SubFolders);
    //Create Enumerator with Folder files inside
    Files = new Enumerator(Folder.Files);
    //Cycle through files in Folder
    while (!Files.atEnd()) {
        //file is a next file
        file = Files.item();
        //put file name into filename
        filename = FSO.GetFileName(file);
        //if filename is our name mask, do the job.
        if (filemask.test(filename)) {
            filelistarray.push(file);
        }
        //move to next file
        Files.moveNext();
    }
    //Cycle through subfolders
    while (!Folders.atEnd()) {
        FindFiles(Folders.item().Path, name, filelistarray);
        //WScript.Echo(Folders.item().Path);
        Folders.moveNext();
    }
}

//Find a name for plugin translation template file from source
function GetPluginName(folder_or_file) {
    var plugin_project_file,
        plugin_project_files = [],
        project = "",
        filename = "";
    
    //check our parameter file or folder?
    if (FSO.FileExists(folder_or_file)) {
        //yes, it's a file, set plugin_project_file as a target
        plugin_project_file = FSO.GetFile(folder_or_file);
    } else {
        //Given parameter is a folder, find project files and put to array
        FindFiles(folder_or_file, "\\.vcxproj$|\\.dpr$", plugin_project_files);
        //if there is nothing found, that's mean this is not a plugin, return from function
        if (!plugin_project_files[0]) return;
        //hope, that project file is first file
        plugin_project_file = plugin_project_files[0];
    }
    //read file into var project
    project = ReadFile(plugin_project_file);
    //find a <ProjectName>%langpackfilename%</ProjectName> from *.vcxproj file
    filename = project.match(/<ProjectName>(.+)<\/ProjectName>/);
    if (filename) {
        //return only first item of regexp
        return filename[1];
    } else {
        //There is no specified ProjectName, thus use filename without extension as our langpack name
        return FSO.GetBaseName(plugin_project_file);
    }
}

//Get MUUID of plugin from source file.
function GetMUUID(folder, array) {
    //first, find necessary file list, we are looking for UNICODE_AWARE function, usually this function are in *.cpp, sometimes in *.c and *.h
    //init fillelist array
    var curfile,
        muuidfilelist = [],
        muuid = "",
        find = /()/,
        i = 0,
        values = [],
        allstrings = "",
        string = "",
        vals = "";
    //search for files in "folder" by mask, put result into muuidfilelist
    FindFiles(folder, "\\.cpp$|\\.c$|\\.h$", muuidfilelist);
    //now we have files, let's put them to Enumerator
    var filesenum = new Enumerator(muuidfilelist);
    //cycle through file list and lookup each file for UNICODE_AWARE
    while (!filesenum.atEnd()) {
        //curfile is our current file in files enumerator
        curfile = filesenum.item();
        //this is a regexp to search UNICODE_AWARE
        find = /(?:UNICODE_AWARE(?:\s*?\|\s*?STATIC_PLUGIN)?,[\s\S]*?\{)(.+?)(?=\}\s{0,2}\})/g;
        //read file fully into var "allstrings"
        allstrings = ReadFile(curfile);
        //search regexp in "allstrings" and put results into var "string"
        string = find.exec(allstrings);
        //if current file have found UNICODE_AWARE, var "string" exists, so parse it.
        if (string) {
            //remove spaces, "0x" and "{", in second [1] item of array "string". RegExp "find" have subregexp (.+?), which are our MUUID in "string[1]"
            vals = string[1].replace(/0x|\{|\x20/g, "");
            //now split values of muuid in "vals" by "," into array "values"
            values = vals.split(",");
            //we get array of values, if length of this array not equal to 12 values (starting from zero, thus length is 11), that's mean we found something else, not MUUID. Log it and quit, if length is 12, so check it and generate MUUID          
            if (values.length == 11) {
                //now check, is there some values, which have omitted zero after 0x, like in alarms: " 0x4dd7762b, 0xd612, 0x4f84, { 0xaa, 0x86, 0x(no_zero_here_)6, 0x8f, 0x17, 0x85, 0x9b, 0x6d}"
                //first value in values have to be 8 bytes, while length less than 8, add leading "0" to values[0],  
                while (values[0].length < 8) {
                    values[0] = "0" + values[0];
                }
                //next two values have to be 4 bytes, adding leading zeros, while length less than 4.
                for (i = 1; i <= 2; i++) {
                    while (values[i].length < 4) {
                        values[i] = "0" + values[i];
                    }
                }
                //other values have to be 2 bytes, same as above, adding zeros
                for (i = 3; i <= 10; i++) {
                    while (values[i].length < 2) {
                        values[i] = "0" + values[i];
                    }
                }
                //Push to array founded #muuid
                muuid = "#muuid {" + values[0] + "-" + values[1] + "-" + values[2] + "-" + values[3] + values[4] + "-" + values[5] + values[6] + values[7] + values[8] + values[9] + values[10] + "}";
            }
        }
        //moving to next file
        filesenum.moveNext();
    }
    //if we didn't find muuid, put alarm into "muuid"
    if (!muuid) {
        muuid = ";#muuid for " + plugin + " not found, please specify manually!";
    }
    //output result into array
    array.push(muuid);
    //log output
    if (log) {
        WScript.Echo(muuid);
    }
}

//read text file, removing all commented text for further processing
function ReadFile(file) {
    //If file zero size, return;
    if (FSO.GetFile(file).Size === 0) return;
    //reading current file
    var file_stream = FSO.GetFile(file).OpenAsTextStream(ForReading, TristateUseDefault);
    //read file fully into var
    var allstrings = file_stream.ReadAll();
    //remove all comments. The text starting with \\ 
    //(but not with ":\\" it's a link like https://miranda-ng.org/ 
    //and ")//" -there is one comment right after needed string)
    //and remove multi-line comments, started with /* and ended with */
    var text = allstrings.replace(/(?:[^\):])(\/{2}.+?(?=$))|(\s\/\*[\S\s]+?\*\/)/mg, ".");
    //close file
    file_stream.Close();
    return text;
}

//Parsing filelist into stringsarray by parsefunction (ParseSourceFile OR ParseRCFile)
function ParseFiles(filelist, stringsarray, parsefunction) {
    var current_strings = 0,
        crap_strings = 0,
        curfile = {},
        filetext = "",
        curfilepath = "";
    //create enumerator filesenum from filelist
    var filesenum = new Enumerator(filelist);
    //cycle through file list
    while (!filesenum.atEnd()) {
        //record into current_strings current length of stringsarray
        current_strings = stringsarray.length;
        //record into crap_strings current length of crap array
        crap_strings = crap.length;
        //curfile is our current file in files enumerator
        curfile = filesenum.item();
        //read file (filtering comments) into filetext
        filetext = ReadFile(curfile);
        //now apply a parsing function to current filetext, and put result into stringsarray
        parsefunction(filetext, stringsarray);
        //string variable to cut out a trunkPath from absolute path
        curfilepath = curfile.Path;
        //if after parsing file our stringsarray length greater then var "current_strings", so parsed file return some strings. Thus, we need add a comment with filename
        if (stringsarray.length > current_strings) {
            stringsarray.splice(current_strings, 0, ";file " + curfilepath.substring(trunkPath.length));
        }
        //do the same for crap array, add a ;file +relative path to file with crap
        if (crap.length > crap_strings) {
            crap.splice(crap_strings, 0, ";file " + curfilepath.substring(trunkPath.length));
        }
        //move to next file
        filesenum.moveNext();
    }
}

//*.RC files line-by-line parser for RC_File, return result into "array"
function ParseRCFile(FileTextVar, array) {
    var string = "",
        onestring = "";
    var find = /^(?!\/{1,2})\s*(CONTROL|(?:DEF)?PUSHBUTTON|[LRC]TEXT|(?:AUTO)?RADIOBUTTON|GROUPBOX|(?:AUTO)?CHECKBOX|CAPTION|MENUITEM|POPUP)\s*"((?:(?:""[^"]+?"")*[^"]*?)*)"\s*?(,|$|\\)/mgi;
    //now make a job, till end of matching regexp
    while ((string = find.exec(FileTextVar)) !== null) {
        // check for some garbage like "List1","Tab1" etc. in *.rc files, we do not need this.
        onestring = string[2].replace(/^(((List|Tab|Tree|Spin|Custom|Slider|DateTimePicker|Radio|Check|HotKey|Progress)\d)|(whiterect|IndSndList|&?[Oo][Kk]|ANSI|APOP|BBS|Bing|CTCP|DCC|Foodnetwork|Google|Google Talk|GPG|ICQ Corp|ID|IP|ISDN|iTunes|Jabber|JID|Miranda|Miranda NG|MRA|mRadio|NewStory|NickServ|OSD|OTR|PCS|PGP|PSK|PUB|S.ms|SASL|SMS|SSL|Steam|Steam Guard|Tox|Twitter|Winamp \(\*\)|Windows|X400|Yahoo|&\w)|(%.(.*%)?))$/g, "");
        // ignore some popup menu craps
        if (string[1] == "POPUP" && onestring.match(/^([a-zA-Z ]*(menu|context|popup(?!s)))|([A-Z][a-z]+([A-Z][a-z]*)+)|(new item)$/g)) {
            continue;
        }
        //if there is double "", replace with single one
        onestring = onestring.replace(/\"{2}/g, "\"");
        //check result. If it does not match [a-z] (no any letter in results, such as "..." or "->") it's a crap, break further actions.
        if (!onestring.match(/[a-z]/i)) {
            onestring = "";
        }
        //if still something in onestring, push to array
        if (onestring) {
            array.push("[" + onestring + "]");
        }
    }
}

//Source files C++ (*.h,*.c,*.cpp) and *.pas,*.dpr,*.inc (Pascal) multiline parser for translations using LPGEN(), LPGENT(), LPGENW(), Translate(), TranslateT(), TranslateW(), _T(), TranslateA_LP(), TranslateW_LP()
function ParseSourceFile(FileTextVar, array) {
        var string = "",
            onestring = "",
            trimedstring = "",
            noslashstring = "",
            nofirstlaststring = "",
            stringtolangpack = "",
            clearstring = "";
    //not store ?: functions LPGEN or LPGENT? or Translate(T or W) or _T, than any unnecessary space \s, than not stored ?: "(" followed by ' or " (stored and used as \1) than \S\s - magic with multiline capture, ending with not stored ?= \1 (we get " or ' after "("), than none or few spaces \x20 followed by )/m=multiline g=global
    //var find= /(?:LPGEN[TW]?|Translate[TW]?||Translate[AW]_LP|_T)(?:\s*?\(\s*?L?\s*)(['"])([\S\s]*?)(?=\1,?\x20*?(?:tmp)?\))/mg;
    //comment previous line and uncomment following line to output templates without _T() function in source files. Too many garbage from _T()..
    var find = /(?:LPGEN[TW]?|Translate[TUW]?|Translate[AUW]_LP)(?:\s*?\(\s*?L?\s*)((?:(?:"[^"\\]*(?:\\[\S\s][^"\\]*)*")\s*)*)(?:\s*?,?\s*?(?:tmp)?\))/gm;
    //now make a job, till end of matching regexp
    while ((string = find.exec(FileTextVar)) !== null) {
        //first, init empty var
        //replace newlines and all spaces and tabs between two pairs of " or ' with the void string ("") in first [1] subregexp ([\S\s]*?), and Delphi newlines "'#13#10+" replace 
        onestring = string[1].replace(/["']?(?:\#13\#10)*?\\?\r*\n(?:(?:\x20|\t)*['"])?/g, "");
        //trim single-line whitespaces - multi-line parsing catches whitespaces after last " in single-line case
        trimedstring = onestring.replace(/[\s]*$/g, "");
        //remove trailing slash from the string. This is a tree item, slash is a crap :)
        noslashstring = trimedstring.replace(/\/(?=$)/g, "");
        //remove first and last "
        nofirstlaststring = noslashstring.slice(1, -1);
        //remove escape slashes before ' and "
        stringtolangpack = nofirstlaststring.replace(/\\(")/g, "$1");
        //if our string still exist, and length at least one symbol
        if (stringtolangpack.length > 0) {
            //brand new _T() crap filtering engine :)
            clearstring = filter_T(stringtolangpack);
            //finally put string into array including cover brackets []
            if (clearstring) {
                array.push("[" + clearstring + "]");
            }
        }
    }
}

//filter _T() function results
function filter_T(string) {
    //filter for exact matched strings
    var filter1 = /^(&?[Oo][Kk]|ANSI|APOP|BBS|Bing|CTCP|DCC|Foodnetwork|Google|GPG|ICQ|ICQ Corp|ID|IP|ISDN|iTunes|Jabber|JID|Miranda|MirandaG15|Miranda NG|mRadio|NewStory|NickServ|OSD|OTR|PCS|PGP|PSK|PUB|SASL|SMS|SSL|SteamID|Steam Guard|Tox|Twitter|Winamp \(\*\)|Windows|X400|Yahoo)$/g;
    //filter string starting from following words
    var filter2 = /^(SOFTWARE\\|SYSTEM\\|http|ftp|UTF-|utf-|TEXT|EXE|exe|txt|css|html|dat[^a]|txt|MS\x20|CLVM|TM_|CLCB|CLSID|CLUI|HKEY_|MButton|BUTTON|WindowClass|MHeader|RichEdit|RICHEDIT|STATIC|EDIT|CList|listbox|LISTBOX|combobox|COMBOBOX|TitleB|std\w|iso-|windows-|<div|<html|<img|<span|<hr|<a\x20|<table|<td|miranda_|kernel32|user32|muc|pubsub|shlwapi|Tahoma|NBRichEdit|CreatePopup|&?[Oo][Kk]|<\/|<\w>|\w\\\w|urn\:|<\?xml|<\!|h\d|\.!\.).*$/g;
    //filter string ending with following words
    var filter3 = /^.+(001|\/value|\*!\*|=)$/g;
    //filter from Kildor
    var filter4 = /^((d\s\w)|\[\/?(\w|url|img|size|quote|color)(=\w*)?\]?|(\\\w)|(%\w+%)|(([\w-]+\.)*\.(\w{2,4}|travel|museum|xn--\w+))|\W|\s|\d)+$/gi;
    //filter from Kildor for remove filenames and paths.
    //var filter5=/^[\w_:%.\\\/*-]+\.\w+$/g;

    //apply filters to our string
    var test1 = filter1.test(string);
    var test2 = filter2.test(string);
    var test3 = filter3.test(string);
    var test4 = filter4.test(string);
    //test5=filter5.test(string);

    //if match (test1) first filter and NOT match other tests, thus string are good, return this string back.
    //if (test1 && !test2 && !test3 && !test4 && !test5) {
    //if (!test1 && !test2 && !test3 && !test4 && !test5) {
    if (!test1 && !test2 && !test3 && !test4) {
        return string;
    } else {
        //in other case, string is a garbage, put into crap array.
        crap.push(string);
    }
    return;
}

function ReadWholeFile(path, codepage) {
    if (codepage === undefined) {
        codepage = "utf-8";
    }
    var adTypeText = 2;
    var bs = WScript.CreateObject("ADODB.Stream");
    bs.Type = adTypeText;
    bs.CharSet = codepage;
    bs.Open();
    bs.LoadFromFile(path);
    var text = bs.ReadText;
    //remove all comments. The text starting with \\ (but not with ":\\" it's a links like https://miranda-ng.org/ and ")//" -there is one comment right after needed string)
    //and remove multi-line comments, started with /* and ended with */
    text = text.replace(/(?:[^\):])(\/{2}.+?(?=$))|(\s\/\*[\S\s]+?\*\/)/mg, ".");
    bs.Close();
    return text;
}

//Parse Version.h file to get one translated string from "Description" and make a plugin template header.
function ParseVersion_h(pluginfolder, array) {
    //cleanup var
    var VersionFile,
        allstrings = "";
    //Let's try default locations of version.h file;
    //Check pluginfolder root.
    if (FSO.FileExists(FSO.BuildPath(pluginfolder, "version.h"))) {
        VersionFile = FSO.BuildPath(pluginfolder, "version.h");
    }
    //Check src\include subfolder of plugin root folder
    if (FSO.FileExists(FSO.BuildPath(pluginfolder, "src\\include\\version.h"))) {
        VersionFile = FSO.BuildPath(pluginfolder, "src\\include\\version.h");
    }
    //Check .\src subfolder
    if (FSO.FileExists(FSO.BuildPath(pluginfolder, "src\\version.h"))) {
        VersionFile = FSO.BuildPath(pluginfolder, "src\\version.h");
    }
    //If we still not found version.h, return
    if (!VersionFile) return;
    //read file fully into var allstrings
    allstrings = ReadWholeFile(VersionFile);
    //define RegExp for defines.
    var filename = /(?:#define\s+_*?FILENAME\s+")(.+)(?=")/m;
    var pluginname = /(?:#define\s+_*?PLUG(?:IN)?_?NAME\s+")(.+)(?=")/i;
    var author = /(?:#define\s+_*?(?:PLUGIN_?)?AUTHORS?\s+")(.+)(?=")/i;
    var MAJOR_VERSION = /(?:#define\s+_*?(?:MAJOR_VERSION|VER_MAJOR)\s+)(\d+)/i;
    var MINOR_VERSION = /(?:#define\s+_*?(?:MINOR_VERSION|VER_MINOR)\s+)(\d+)/i;
    var RELEASE_NUM = /(?:#define\s+_*?(?:RELEASE_NUM|VER_REVISION|VER_RELEASE)\s+)(\d+)/i;
    var BUILD_NUM = /(?:#define\s+_*?(?:BUILD_NUM|VER_BUILD)\s+)(\d+)/i;
    var VERSION_STRING = /(?:#define\s+_*?VERSION_STRING\s+")([\d\.]+)\"/i;
    var description = /(?:#define\s+_*?(?:PLUGIN_|MTEXT_)?DESC(?:RIPTION|_STRING)?\s+")(.+)(?=")/i;
    //exec RegExps
    filename = filename.exec(allstrings);
    pluginname = pluginname.exec(allstrings);
    MAJOR_VERSION = MAJOR_VERSION.exec(allstrings);
    MINOR_VERSION = MINOR_VERSION.exec(allstrings);
    RELEASE_NUM = RELEASE_NUM.exec(allstrings);
    BUILD_NUM = BUILD_NUM.exec(allstrings);
    VERSION_STRING = VERSION_STRING.exec(allstrings);
    author = author.exec(allstrings);
    description = description.exec(allstrings);

    //add a header start mark
    array.push(";============================================================");
    //push results of regexp vars into array
    if (filename) {
        array.push(";  File: " + filename[1]);
    } else {
        array.push(";  File: " + plugin + ".dll");
    }
    if (pluginname) {
        array.push(";  Plugin: " + pluginname[1]);
    } else {
        array.push(";  Plugin: " + plugin);
    }
    if (VERSION_STRING) {
        array.push(";  Version: " + VERSION_STRING[1]);
    }
    if (MAJOR_VERSION && !VERSION_STRING) {
        array.push(";  Version: " + MAJOR_VERSION[1] + "." + MINOR_VERSION[1] + "." + RELEASE_NUM[1] + "." + BUILD_NUM[1]);
    }
    if (!MAJOR_VERSION && !VERSION_STRING) {
        array.push(";  Version: x.x.x.x");
    }
    if (author) {
        array.push(";  Authors: " + fixHexa(author[1]));
    } else {
        array.push(";  Authors: ");
    }
    //add a header end mark
    array.push(";============================================================");
    if (description) {
        array.push("[" + description[1] + "]");
    }
}

//Replaces \x?? hex codes with their char representation
function fixHexa(string) {
    return string.replace(/\\x([a-fA-F0-9]{2})" "/g, function () {
        return String.fromCharCode(parseInt(arguments[1], 16));
    });
}

//Removes duplicates, not mine, found at http://dreaminginjavascript.wordpress.com/2008/08/22/eliminating-duplicates/
function eliminateDuplicates(arr) {
    var i,
        len = arr.length,
        out = [],
        obj = {};

    for (i = 0; i < len; i++) {
        obj[arr[i]] = 0;
    }
    for (i in obj) {
        out.push(i);
    }
    return out;
}

//Output array of strings into file
function WriteToFile(array, langpack) {
    var i = 0,
        len = 0;
    //Create file, overwrite if exists
    var langpackfile = FSO.CreateTextFile(langpack, overwritefile, unicode);
    //Finally, write strings from array to file
    len = array.length - 1;
    for (i = 0; i <= len; i++) {
        langpackfile.WriteLine(array[i]);
    }
    //Close file
    langpackfile.Close();
}

//Write UTF-8 file
function WriteToUnicodeFile(array, langpack) {
    var i = 0,
        len = 0;
    stream.Open();
    len = array.length - 1;
    for (i = 0; i <= len; i++) {
        stream.WriteText(array[i] + "\r\n");
    }
    stream.SaveToFile(langpack, 2);
    stream.Close();
}

//Write file as UTF-8 without BOM
function WriteToUnicodeFileNoBOM(array, filename) {
    var UTFStream = WScript.CreateObject("ADODB.Stream");
    var BinaryStream = WScript.CreateObject("ADODB.Stream");
    var i = 0,
        len = 0,
        adTypeBinary = 1,
        adTypeText = 2,
        adModeReadWrite = 3,
        adSaveCreateOverWrite = 2;

    UTFStream.Type = adTypeText;
    UTFStream.Mode = adModeReadWrite;
    UTFStream.Charset = "utf-8";
    UTFStream.Open();
    
    len = array.length - 1;
    for (i = 0; i <= len; i++) {
        UTFStream.WriteText(array[i] + "\r\n");
    }

    UTFStream.Position = 3; // skip BOM
    BinaryStream.Type = adTypeBinary;
    BinaryStream.Mode = adModeReadWrite;
    BinaryStream.Open();

    // Strips BOM (first 3 bytes)
    UTFStream.CopyTo(BinaryStream);

    BinaryStream.SaveToFile(filename, adSaveCreateOverWrite);
    BinaryStream.Flush();
    BinaryStream.Close();
    UTFStream.Close();
}
