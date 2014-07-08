//*************************************************************************************************************************************************************************** *//
//* Name:     translate.js                                                                                                                                                    *//
//* Language: JScript                                                                                                                                                         *//
//* Function: Parse Miranda-NG translation templates and get translated strings                                                                                               *//
//* Author:   BasiL                                                                                                                                                           *//
//* Usage:    cscript /nologo translate.js  to run generation in batches                                                                                                      *//
//* Usage:    cscript /nologo translate.js /log:"yes" to enable console logging                                                                                               *//
//* Usage:    cscript /nologo translate.js /plugin:"path\file" for one template                                                                                               *//
//* Usage:    cscript /nologo translate.js /path:"path\to\folder" folder with .\Plugins, .\Weather subfolders and =CORE=.txt file                                             *//
//* Usage:    cscript /nologo translate.js /dupes:"path\=dupes=.txt" use dupes file                                                                                           *//
//* Usage:    cscript /nologo translate.js /sourcelang:"language" instead of /path param if your .\Plugins, .\Weather, =CORE=.txt and langpack_%lang%.txt in trunk .\langpacks*//
//* Usage:    cscript /nologo translate.js /out:"path\folder" output result to folder                                                                                         *//
//* Usage:    cscript /nologo translate.js /outfile:"path\file" output result to one file                                                                                     *//
//* Usage:    cscript /nologo translate.js /langpack:"path\lang.txt" - Full langpack                                                                                          *//
//* Usage:    cscript /nologo translate.js /noref:"yes" - remove ref. ";file path\file"                                                                                       *//
//* Usage:    cscript /nologo translate.js /untranslated:"yes|path"  untranslated-only strings output to separated files                                                      *//
//* Usage:    cscript /nologo translate.js /popuntranslated:"yes"  remove untranslated string and empty line below from output files                                          *//
//* Usage:    cscript /nologo translate.js /sourcelang:"russian" /release:"path\file"                                                                                         *//
//* Note:     script will use following sequense to find a translation for string:                                                                                            *//
//* 1) Try to get translation from a same file name. Example: /langpack/english/plugin/TabSRMM.txt strings will be checked in file named TabSRMM.txt in folder from /path:    *//
//* if you specify a "path" - /path:"path\folder", so look in path\folder\TabSRMM.txt                                                                                         *//
//* 2) If not find in step 1), check a string in file DUPES, specified in /dupes parameter                                                                                    *//
//* 3) If still not found, try to find trasnlation in =CORE=.txt file                                                                                                         *//
//* 4) Still no luck? Well, check a /langpack:"/path/lang.txt" as a last place.                                                                                               *//
//* Example1:  cscript /nologo translate.js /langpack:"path\lang.txt" /path:"path/german" will translate english templates using .\plugins translation from path\german and if*//
//* translation not found, try to find translation in path\lang.txt                                                                                                           *//
//* Example2:  cscript /nologo translate.js /plugin:"path\file" /langpack:"path\lang.txt" will translate path\file using translation from path\lang.txt                       *//
//* Example3:  cscript /nologo translate.js /langpack:"path\lang.txt" /outfile:"path\file" will translate all /english/* templates using lang.txt & out langpack in path\file *//
//* Example4:  cscript /nologo translate.js /sourcelang="Russian" /outfile:"path\file" will translate all /english/* tempaltes using files =CORE=.txt, =DUPES=.txt,           *//
//* lanpack_Russian.txt from ./langpacks/Russian/ folder, including /langpacks/Russian/Plugins/*                                                                              *//
//* Example5:  cscript translate.js /sourcelang:"Russian" /release:"Langpack_rusian.txt" will output a "release" version of langpack, using files in \langpacks\russian\ with *//
//* =HEAD=.txt, but "clean" - no file reference and no untranslated strings inside                                                                                            *//
//*****************************************************************************************************************************************************************************//

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
//output translated templates in separated files by default
var outfile=false;
//do not remove reference to source file, where we found a translation string
var noref=false;
//disable output untranslated_* files by default
var untranslated=false;
//include untranslated strings and empty line below in output files by default
var popuntranslated=false;
//disable release output by default
var release=false
//Path variables
var scriptpath=FSO.GetParentFolderName(WScript.ScriptFullName);
//crazy way to get path two layers upper "\tools\lpgen\"
var trunk=FSO.GetFolder(FSO.GetParentFolderName(FSO.GetParentFolderName(scriptpath)));
//path to "English" langpack
var langpackenglish="\\langpacks\\english\\"
//stream - our variable for output UTF-8 files with BOM
var stream= new ActiveXObject("ADODB.Stream");
//stream var tune
stream.Type = 2; // text mode
stream.Charset = "utf-8";
//init translate dictionaries
CoreTranslateDict=WScript.CreateObject("Scripting.Dictionary");
DupesTranslateDict=WScript.CreateObject("Scripting.Dictionary");
LangpackTranslateDict=WScript.CreateObject("Scripting.Dictionary");
//init arrays
Translated_Core_Array=new Array;
UnTranslated_Core_Array=new Array;
full_langpack_array=new Array;
release_array=new Array;

//*********************************************************************************//
//                         Checking command line parameters                       *//
//*********************************************************************************//

// if console param /log: specified, put it to var log. To enable log, specify /log:"yes"
if (WScript.Arguments.Named.Item("log")) log=true;

// if console param /noref: specified, put it to var noref. To remove ref's to files, specify /noref:"yes"
if (WScript.Arguments.Named.Item("noref")) noref=true;

// if console param /untranslated: specified, put it to var untranslated. To output untranslated_*  files, specify /untranslated:"yes", or specify a path to output untranslated files folder
if (WScript.Arguments.Named.Item("untranslated")) {
    untranslated=true;
    UnTranslatedPath=WScript.Arguments.Named.Item("untranslated")
    if (WScript.Arguments.Named.Item("untranslated").toLowerCase()!="yes") {
        CreateFldr(UnTranslatedPath);
        }
    };

//if console param /popuntranslated: specified, put it to var popuntranslated
if (WScript.Arguments.Named.Item("popuntranslated")) popuntranslated=true;

// if console pararm /outpfile:"\path\filename.txt" given, put it to var outfile.
if (WScript.Arguments.Named.Item("outfile")) {
    outfile=true;
    //path to full langpack file
    full_langpack_file=WScript.Arguments.Named.Item("outfile");
    }
// if console pararm /release:"\path\filename.txt" given, put it to var release.
if (WScript.Arguments.Named.Item("release")) {
    release=true;
    //path to full langpack file
    release_langpack_file=WScript.Arguments.Named.Item("release");
    }

// if param /out specified, build a path and put it into var.
if (WScript.Arguments.Named.Item("out")) {
    var out=WScript.Arguments.Named.Item("out");
    var OutPlugins=FSO.BuildPath(out,"Plugins");
    var OutWeather=FSO.BuildPath(out,"Weather");
    CreateFldr(out);
    CreateFldr(OutPlugins);
    CreateFldr(OutWeather);
    };
    
//If script run by double click, open choose folder dialog to choose plugin folder to parse. If Cancel pressed, quit script.
if (WScript.FullName.toLowerCase().charAt(WScript.FullName.length - 11)=="w") {
   WScript.Echo("Please run from command line!");
   WScript.Quit();
}

//when /sourcelang specified, setup all source files already existed in trunk. Useful for running translate.js from trunk.
// Currently seldom languages have same files structure, thus it is much more easier to just specify a language folder name, instead of specifying /path, /dupes, /langpack.
if (WScript.Arguments.Named.Item("sourcelang")) {
    var sourcelang=WScript.Arguments.Named.Item("sourcelang");
    var langpack_path=FSO.BuildPath(FSO.BuildPath(trunk,"langpacks"),sourcelang);
    var translated_plugins=FSO.BuildPath(langpack_path,"Plugins");
    var translated_weather=FSO.BuildPath(langpack_path,"Weather");
    var translated_core=FSO.BuildPath(langpack_path,"=CORE=.txt");
    var translated_dupes=FSO.BuildPath(langpack_path,"=DUPES=.txt");
    var langpack_head=FSO.BuildPath(langpack_path,"=HEAD=.txt");
    var translated_langpack=FSO.BuildPath(langpack_path,("langpack_"+sourcelang+".txt"));
    if (log) WScript.Echo("Translating to "+sourcelang);
}

//when /plugin: specified, parse only this path and quit
if (WScript.Arguments.Named.Item("plugin")) {
    //First, generate DB of translations from Core and Dupes files
    checkparams();
    GenerateDictionaries();
    //plugin from command line:
    var cmdline_file=new String(WScript.Arguments.Named.Item("plugin"));
    //init array for our file translation and untranslated strings
    var cmdline_file_array=new Array;
    var cmdline_untranslated_array=new Array;
    //Output filename variable
    var traslated_cmdline_file=new String(FSO.BuildPath(scriptpath,FSO.GetFileName(cmdline_file)));
    var untranslated_cmdline_file=new String(FSO.BuildPath(scriptpath,"untranslated_"+FSO.GetFileName(cmdline_file)));
    //logging
    if (log) WScript.Echo("translating "+cmdline_file);
    //Call TranslateTemplateFile for path specified in command line argument /path:"path/to/template", output result to "scriptpath"
    TranslateTemplateFile(WScript.Arguments.Named.Item("plugin"),cmdline_file_array,cmdline_untranslated_array);
    //Output results to scriptpath folder.
    WriteToUnicodeFile(cmdline_file_array,traslated_cmdline_file);
    if (log) WScript.Echo("Translated file:     "+traslated_cmdline_file);
    //if there is something untranslated in cmdline_untranslated_array, output to file
    if (cmdline_untranslated_array.length>0) {
        WriteToUnicodeFile(cmdline_untranslated_array,untranslated_cmdline_file);
        if (log) WScript.Echo("Untranslated file:   "+traslated_cmdline_file);
        }
    //We are done, quit.
    WScript.Quit();
}


//*********************************************************************************//
//                                    Main part                                   *//
//*********************************************************************************//
//first, check we have files with translated strngs specified.
checkparams();
if (log) WScript.Echo("Translation begin");

//Add a =HEAD=.txt into FullLangpack Array and release array if file exist and /out or /release specified.
if ((outfile || release) && FSO.FileExists(langpack_head)) {
    //open file
    stream.Open();
    stream.LoadFromFile(langpack_head);
    //read file into var
    var headertext=stream.ReadText();
    full_langpack_array.push(headertext);
    release_array.push(headertext);
    stream.Close();
    }

//Generate translation dictionaries from /path, /dupes and /langpack files.
GenerateDictionaries ();

if (log) WScript.Echo("Translating Core");
//Call function for translate core template
TranslateTemplateFile(FSO.BuildPath(trunk,langpackenglish+"=CORE=.txt"),Translated_Core_Array,UnTranslated_Core_Array);
//output core file, if /out specified.
if (out) OutputFiles(Translated_Core_Array,UnTranslated_Core_Array,"","=CORE=.txt")


//Init array of template files
TemplateFilesArray=new Array;
//Init array of weather.ini translation files
WeatherFilesArray=new Array;
//Find all template files and put them to array
FindFiles(FSO.BuildPath(trunk,langpackenglish+"plugins\\"),"\\.txt$",TemplateFilesArray)
//Find all weather.ini template files and add them into array
FindFiles(FSO.BuildPath(trunk,langpackenglish+"Weather\\"),"\\.txt$",WeatherFilesArray)
//Build enumerator for each file array
TemplateFilesEnum=new Enumerator(TemplateFilesArray);
WeatherFilesEnum=new Enumerator(WeatherFilesArray);
//Run processing files one-by-one;
ProcessFiles(TemplateFilesEnum);

//if output to one langpack file, write a final array Translated_Core_Array into UTF-8 file with BOM
if (outfile) {
    WriteToUnicodeFile(full_langpack_array,full_langpack_file);
    if (log) WScript.Echo("Langpack file in "+full_langpack_file);
}
//if /release specified, output array into file
if (release) {
    WriteToUnicodeFile(release_array,release_langpack_file);
    if (log) WScript.Echo("Release langpack file in "+release_langpack_file);
}
if (log) WScript.Echo("Translation end");


//*********************************************************************************//
//                                    Functions                                   *//
//*********************************************************************************//

//Process files one-by-one using enummerator
function ProcessFiles (FilesEnum) {
//cycle through file list
 while (!FilesEnum.atEnd()) {
     //intit Array with translated strings and untranslated stings
     TranslatedTemplate=new Array;
     UnTranslatedStrings=new Array;
     //curfile is our current file in files enumerator
     curfile=FilesEnum.item();
     //Log output to console
     if (log) WScript.Echo("translating:     "+curfile);
     //now put strings from template and translations into array
     TranslateTemplateFile(curfile,TranslatedTemplate,UnTranslatedStrings);
     //output files, if /out specified
     if (out) OutputFiles(TranslatedTemplate,UnTranslatedStrings,FSO.GetBaseName(FSO.GetParentFolderName(curfile)),FSO.GetFileName(curfile))
     //move to next file
     if (FSO.GetBaseName(curfile)=="Weather") {
        ProcessFiles(WeatherFilesEnum);
        }
     FilesEnum.moveNext();
    };
}

//Create Folder function, if folder does not exist.
function CreateFldr(FolderPathName) {
    if (!FSO.FolderExists(FolderPathName)) {
        var CreatedFolder=FSO.CreateFolder(FolderPathName);
        if (log) WScript.Echo("Folder created: "+CreatedFolder);}
}
//output to files. Checking params, and output file(s).
function OutputFiles(TranslatedArray,UntranslatedArray,FolderName,FileName) {
    //clear var outpath
    var outpath;
    //outpath is a /out:"path" + FolderName
    outpath=FSO.BuildPath(out,FolderName);
    //define default path to files in "langpacks\english\plugins"
    TraslatedTemplateFile=trunk+langpackenglish+"plugins\\translated_"+FileName
    UnTranslatedFile=trunk+langpackenglish+"plugins\\untranslated_"+FileName
    
    //redefine path to files, if /out specified
    if (out) {
        TraslatedTemplateFile=FSO.BuildPath(outpath,FileName);
        UnTranslatedFile=out+"\\untranslated_"+FileName;
        }
        
    //redefine path to files, if FileName is a =CORE=.txt
    if (FileName=="=CORE=.txt") {
        TraslatedTemplateFile=trunk+langpackenglish+"translated_"+FileName;
        UnTranslatedFile=trunk+langpackenglish+"untranslated_"+FileName;
        if (out) {
            // if /out:"/path/folder" specified redefine path of translated and untranslated =CORE=.txt file to parent folder of specified path
            TraslatedTemplateFile=FSO.BuildPath(outpath,FileName);
            // if /untranslated:"yes" specified, redefine untranslated core to parent folder, same as above.
            UnTranslatedFile=FSO.BuildPath(outpath,"untranslated_"+FileName);
            }
        // if /untralsated:"path" specified, redefine path to untranslated "=CORE=.txt"
        if (untranslated && (UnTranslatedPath!="yes")) UnTranslatedFile=FSO.BuildPath(UnTranslatedPath,FileName);
        }
    
    // output translated file if /out and /outfile ommited, or if /out specified
    if ((!out && !outfile) || out) {
        if (log) WScript.Echo("Output to file:  "+TraslatedTemplateFile);
        WriteToUnicodeFile(TranslatedArray,TraslatedTemplateFile);
        }
    
    //Write untranslated array into file, if /untranslated specified and there is something in array
    if (untranslated & UntranslatedArray.length>0) {
        //redefine Untranslated file path and name, if /untranslated:"/path/" specified
        if (UnTranslatedPath!="yes") UnTranslatedFile=UnTranslatedPath+"\\"+FileName;
        if (log) WScript.Echo("Untranslated in: "+UnTranslatedFile);
        WriteToUnicodeFile(UntranslatedArray,UnTranslatedFile);
        }
}

//when /sourcelang: and /path: are NOT specified, thus we don't have any langpack file(s) to get translated strings. Thus all other job are useless
function checkparams() { 
if (!WScript.Arguments.Named.Item("langpack") & !WScript.Arguments.Named.Item("path") & !sourcelang)  {
    WScript.Echo("you didn't specify /langpack:\"/path/to/langpack.txt\", /path:\"/path/to/plugnis/\"  or /sourcelang:\"language\" parameter, there is no files with translated strings!");
    WScript.Quit();
    };
}

//Check file existense. If file not found, quit.
function CheckFileExist(file) {
    if (!FSO.FileExists(file) && log) {
        WScript.Echo("Can't find "+file)
        };
}

//Generate DB with translations from Core and Dupes files
function GenerateDictionaries () {
//if /sourcelang:"language" specified, use it for generate dicitionaries
if (sourcelang) {
    CheckFileExist(translated_core);
    CheckFileExist(translated_dupes);
    CheckFileExist(translated_langpack);
    GenerateTransalteDict(translated_core,CoreTranslateDict);
    GenerateTransalteDict(translated_dupes,DupesTranslateDict);
    GenerateTransalteDict(translated_langpack,LangpackTranslateDict);
}
//process a dictionaries creation with switch-specified pathes
if (WScript.Arguments.Named.Item("path")) {
    //Check file =CORE=.txt and =DUPES=.txt exist in /path:"path" or not
    PathToCore=FSO.BuildPath(WScript.Arguments.Named.Item("path"),"\\=CORE=.txt")
    PathToDupes=FSO.BuildPath(WScript.Arguments.Named.Item("path"),"\\=DUPES=.txt")
    CheckFileExist(PathToCore);
    CheckFileExist(PathToDupes);
    //Generate dictionanries
    GenerateTransalteDict(PathToCore,CoreTranslateDict);
    GenerateTransalteDict(PathToDupes,DupesTranslateDict);
    }
if (WScript.Arguments.Named.Item("dupes")) {
    CheckFileExist(WScript.Arguments.Named.Item("dupes"));
    GenerateTransalteDict(WScript.Arguments.Named.Item("dupes"),DupesTranslateDict);
    }
if (WScript.Arguments.Named.Item("langpack")) {
    CheckFileExist(WScript.Arguments.Named.Item("langpack"));
    GenerateTransalteDict(WScript.Arguments.Named.Item("langpack"),LangpackTranslateDict);
    }
}

//Generate Dictionary with english sting + translated string from file
function GenerateTransalteDict (file,dictionary) {
//if file does not exist, it's a core, we do not need do the job again, so return.
if (!FSO.FileExists(file)) return;
//open file
stream.Open();
stream.LoadFromFile(file);
//read file into var
var translatefiletext=stream.ReadText();
//"find" - RegularExpression, first string have to start with [ and end with]. Next string - translation
var find=/(^\[.+?\](?=$))\r?\n(^(?!;file|\r|\n).+?(?=$))/mg;
//While our "find" RegExp return a results, add strings into dictionary.
while ((string = find.exec(translatefiletext)) != null) {
    //first, init empty var
    var string;
    //first match as original string [....], is a key of dictionary, second match is a translation - item of key in dictionary 
    var key=string[1];
    var item=string[2];
    //ignore "translations" (wrongly parsed untranslated strings) begining and ending with []
    if (item.match(/^\[.*\]$/))
        continue;
    //add key-item pair into dictionary, if not exists already
    if (!dictionary.Exists(key))
        dictionary.Add(key,item);
    // add key-item pair for case-insensitive match (with prefix so it won't interfere with real keys)
    lowerKey = "__lower__" + key.toLowerCase();
    if (!dictionary.Exists(lowerKey))
        dictionary.Add(lowerKey,item);
    //use also different variants of Miranda name for translations from old langpacks
    key = key.replace("Miranda IM", "Miranda NG");
    item = item.replace("Miranda IM", "Miranda NG");
    if (!dictionary.Exists(key))
        dictionary.Add(key,item);   
    key = key.replace("Miranda NG", "Miranda");
    item = item.replace("Miranda NG", "Miranda");
    if (!dictionary.Exists(key))
        dictionary.Add(key,item);
    }
//close file
stream.Close();
}

//Generate array with stirngs from translation template, adding founded translation, if exist.
function TranslateTemplateFile(Template_file,translated_array,untranslated_array) {
 //Init PluginTranslate Dictionary from plugins translate file
 var PluginTranslateDict=WScript.CreateObject("Scripting.Dictionary");
 //if /sourcelang specified, use it for search plugin translation.
 if (sourcelang) GenerateTransalteDict(FSO.BuildPath(langpack_path,(FSO.GetBaseName(FSO.GetParentFolderName(Template_file))+"\\"+FSO.GetFileName(Template_file))),PluginTranslateDict);
 // if /path:"" specified, this is a folder with plugin translations, use it to find out our translation.
 if (WScript.Arguments.Named.Item("path")) {
    //Generate PluginTranslate Dictionary
    GenerateTransalteDict(FSO.BuildPath(WScript.Arguments.Named.Item("path"),(FSO.GetBaseName(FSO.GetParentFolderName(Template_file))+"\\"+FSO.GetFileName(Template_file))),PluginTranslateDict);
 }
 //If file zero size, return;
 if (FSO.GetFile(Template_file).Size==0) return;  
 //access file
 stream.Open();
 stream.LoadFromFile(Template_file);
 //Reading line-by-line
 while (!stream.EOS) {
     //clear up variable
     englishstring="";
     //read on line
     var line=stream.ReadText(-2);
     //If we need reference to "; file source\file\path" in template or langpack, put into array every line 
     if (!noref) translated_array.push(line);
     //RegExp matching strings, starting from ";file"
     refline=line.match(/^;file.+/);
     //RegExp for match a =CORE=.txt header line "Miranda Language Pack Version 1". If /noref specified, remove this line as well.
     headerline=line.match(/^Miranda Language Pack Version 1$/)
     //if /noref enabled, check string and if not matched, add to array
     if (noref && (!refline && !headerline)) translated_array.push(line);
     //same for /release
     if (release && (!refline && !headerline)) release_array.push(line);
     //find string covered by[] using regexp
     englishstring=line.match(/\[.+\]/);
     //If current line is English string covered by [], try to find translation in global db
     if (englishstring) {
        var cycle = -1;
        var found = false;
        var search = line;
        
        while (cycle<1 && !found) {
            cycle = cycle+1;
            if (cycle==1) {
                // second cycle, try to be case-insensitive now
                search = "__lower__" + line.toLowerCase();
            }

            //uncomment next string for more verbose log output
            //if (log) WScript.Echo("lookin' for "+englishstring);
            //firstly find our string exist in Plugin translate DB dictionary
            if (PluginTranslateDict.Exists(search)) {
                //yes, we have translation, put translation into array
                translated_array.push(PluginTranslateDict.Item(search));
                //add translation to release array
                release_array.push(PluginTranslateDict.Item(search));
                found = true;
            } else {
                //if we do not found string in plugin translation, check Dupes and if found, put to array
                if (DupesTranslateDict.Exists(search)) {
                    translated_array.push(DupesTranslateDict.Item(search));
                    release_array.push(DupesTranslateDict.Item(search));
                    found = true;
                } else {
                    //not found in dupes? Check CORE
                    if (CoreTranslateDict.Exists(search)) {
                        translated_array.push(CoreTranslateDict.Item(search));
                        release_array.push(CoreTranslateDict.Item(search));
                        found = true;
                    } else {
                        //still no luck? Check Langpack...
                        if (LangpackTranslateDict.Exists(search)) {
                            translated_array.push(LangpackTranslateDict.Item(search));
                            release_array.push(LangpackTranslateDict.Item(search));
                            found = true;
                        }
                    }
                }
            }
        }

        if (!found) {
            //no translation found, put empty line if popuntranslated disabled
            if (!popuntranslated) translated_array.push("");
            //add to untranslated array
            untranslated_array.push(line);
            //remove from release, no translation found.
            release_array.pop();
			//remove from translated array if popuntranslated enabled.
			if (popuntranslated) translated_array.pop();
        }
    }
 }
 //closing file
 stream.Close();
 //if we will output one file only, concatenate array
 if (outfile) full_langpack_array=full_langpack_array.concat(translated_array);
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

//Write UTF-8 file
function WriteToUnicodeFile(array,langpack) {
stream.Open();
for (i=0;i<=array.length-1;i++) stream.WriteText(array[i]+"\r\n");
stream.SaveToFile(langpack, 2);
stream.Close();
}