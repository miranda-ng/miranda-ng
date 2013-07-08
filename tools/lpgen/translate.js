//*****************************************************************************************//
//* Name:      translate.js                                                               *//
//* Language:  JScript                                                                    *//
//* Function:  Parse Miranda-NG translation templates and get translated strings          *//
//* Author:    BasiL                                                                      *//
//* Usage:     cscript /nologo translate.js  to run generation in batches                 *//
//* Usage:     cscript /nologo translate.js /log:"yes" to enable console logging          *//
//* Usage:     cscript /nologo translate.js /plugin:"path\file" for one template          *//
//* Usage:     cscript /nologo translate.js /path:"path\plugins" translated plugins folder*//
//* Usage:     cscript /nologo translate.js /core:"path\=core=.txt" use core file         *//
//* Usage:     cscript /nologo translate.js /dupes:"path\=dupes=.txt" use dupes file      *//
//* Usage:     cscript /nologo translate.js /sourcelang:"language" instead of /path+/core *//
//* Usage:     cscript /nologo translate.js /out:"path\plugins" output result to folder   *//
//* Usage:     cscript /nologo translate.js /outfile:"path\file" output result to one file*//
//* Usage:     cscript /nologo translate.js /langpack:"path\lang.txt" - Full langpack     *//
//* Usage:     cscript /nologo translate.js /noref:"yes" - remove ref. ";file path\file"  *//
//* Usage:     cscript /nologo translate.js /untranslated:"yes|path"  untranslated strings*//
//* Note:      script will use following sequense to find a translation for string:       *//
//* 1) Try to get translation from a same file name. Example: /langpack/english/plugin/   *//
//* /TabSRMM.txt strings will be checked in file named TabSRMM.txt in folder from /path:  *//
//* if you specify a "path" - /path:"path\folder", so look in path\folder\TabSRMM.txt     *//
//* 2) If not find in step 1), check a string in file DUPES, specified in /dupes parameter*//
//* 3) If still not found, try to find trasnlation in /core file                          *//
//* 4) Still no luck? Well, check a /langpack:"/path/lang.txt" as a last place.           *//
//* Example1:  cscript /nologo translate.js /core:"path\lang.txt" /path:"path/german"     *//
//* will translate english templates using plugins translation from path\german and if    *//
//* translation not found, try to find translation in path\lang.txt                       *//
//* Example2:  cscript /nologo translate.js /plugin:"path\file" /langpack:"path\lang.txt" *//
//* will translate path\file using translation from path\lang.txt                         *//
//* Example3:  cscript /nologo translate.js /langpack:"path\lang.txt" /outfile:"path\file"*//
//* will translate all /english/* templates using lang.txt & output langpack in path\file *//
//* Example4:  cscript /nologo translate.js /sourcelang="Russian" /outfile:"path\file"    *//
//* will translate all /english/* tempaltes using files =CORE=.txt, =DUPES=.txt, lanpack_R*//
//* ussian.txt from ./langpacks/Russian/ folder, including /langpacks/Russian/Plugins/*   *//
//*****************************************************************************************//

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
//do not remove refference to source file, where we found a translation string
var noref=false;
//disable output untranslated_* files by default
var untranslated=false;
//Path variables
var scriptpath=FSO.GetParentFolderName(WScript.ScriptFullName);
//crazy way to get path two layers upper "\tools\lpgen\"
var trunk=FSO.GetFolder(FSO.GetParentFolderName(FSO.GetParentFolderName(scriptpath)));
//stream - our variable for output UTF-8 files with BOM
var stream= new ActiveXObject("ADODB.Stream");
//stream var tune
stream.Type = 2; // text mode
stream.Charset = "utf-8";
//init translate dictionaries
CoreTranslateDict=WScript.CreateObject("Scripting.Dictionary");
DupesTranslateDict=WScript.CreateObject("Scripting.Dictionary");
LangpackTranslateDict=WScript.CreateObject("Scripting.Dictionary");

//*********************************************************************************//
//                         Checking command line parameters                       *//
//*********************************************************************************//

// if console param /log: specified, put it to var log. To enable log, specify /log:"yes"
if (WScript.Arguments.Named.Item("log")) log=true;

// if console param /noref: specified, put it to var noref. To remove reff's to files, specifry /noref:"yes"
if (WScript.Arguments.Named.Item("noref")) noref=true;

// if console param /untranslated: specified, put it to var untranslated. To output untranslated_*  files, specifry /untranslated:"yes", or specify a path to output untranslated files folder
if (WScript.Arguments.Named.Item("untranslated")) {
    untranslated=true;
    UnTranslatedPath=WScript.Arguments.Named.Item("untranslated")
    if (WScript.Arguments.Named.Item("untranslated").toLowerCase()!="yes") {
        if (!FSO.FolderExists(UnTranslatedPath)) {
        WScript.Echo("Output folder "+UnTranslatedPath+" does not exist!\nPlease, create it first!");
        WScript.Quit();}
        }
    };

// if console pararm /outpfile:"\path\filename.txt" given, put it to var outfile.
if (WScript.Arguments.Named.Item("outfile")) {
    outfile=true;
    full_langpack_file=WScript.Arguments.Named.Item("outfile");
    }
// if param /out specified, build a path and put it into var.
if (WScript.Arguments.Named.Item("out")) {
    var out=WScript.Arguments.Named.Item("out");
    if (!FSO.FolderExists(out)) {
        WScript.Echo("Output folder "+out+" does not exist!\nPlease, create it first!");
        WScript.Quit();}
    };
    
//If script run by double click, open choose folder dialog to choose plugin folder to parse. If Cancel pressed, quit script.
if (WScript.FullName.toLowerCase().charAt(WScript.FullName.length - 11)=="w") {
   WScript.Echo("Please run from command line!");
   WScript.Quit();
}

//when /sourcelang specified, setup all source files already existed in trunk. Usefull for running translate.js from trunk. Currenly seldom languages have same files structure, thus it is much more easier to just specify a language folder name, instead of specifying /core, /path, /dupes, /langpack.
if (WScript.Arguments.Named.Item("sourcelang")) {
    var sourcelang=WScript.Arguments.Named.Item("sourcelang");
    var langpack_path=FSO.BuildPath(FSO.BuildPath(trunk,"langpacks"),sourcelang);
    var translated_plugins=FSO.BuildPath(langpack_path,"Plugins");
    var translated_core=FSO.BuildPath(langpack_path,"=CORE=.txt");
    var translated_dupes=FSO.BuildPath(langpack_path,"=DUPES=.txt");
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
//first, check we have files with translated stirngs specified.
checkparams();
if (log) WScript.Echo("Translation begin");

//Generate translation dictionaries from /core, /dupes and /langpack files.
GenerateDictionaries ();

//Array for translated core & full langpack file
Translated_Core_Array=new Array;
UnTranslated_Core_Array=new Array;
full_langpack_array=new Array;
if (log) WScript.Echo("Translating Core");
//Call function for translate core template
TranslateTemplateFile(FSO.BuildPath(trunk,"langpacks\\english\\=CORE=.txt"),Translated_Core_Array,UnTranslated_Core_Array);
//output core file, if needed.
OutputFiles(Translated_Core_Array,UnTranslated_Core_Array,"=CORE=.txt")
//Init array of template files
TemplateFilesArray=new Array;
//Find all template files and put them to array
FindFiles(FSO.BuildPath(trunk,"langpacks\\english\\plugins\\"),"\\.txt$",TemplateFilesArray)
//create enumerator filesenum from filelist
filesenum=new Enumerator(TemplateFilesArray);
//cycle through file list
 while (!filesenum.atEnd()) {
     //intit Array with translated strings and untranslated stings
     TranslatedTemplate=new Array;
     UnTranslatedStrings=new Array;
     //curfile is our current file in files enumerator
     curfile=filesenum.item();
     //Log output to console
     if (log) WScript.Echo("translating:     "+curfile);
     //now put strings from template and translations into array
     TranslateTemplateFile(curfile,TranslatedTemplate,UnTranslatedStrings);
     //output files, if need
     OutputFiles(TranslatedTemplate,UnTranslatedStrings,FSO.GetFileName(curfile))
     //move to next file
     filesenum.moveNext();
    };
//if output to one langpack file, write a final array Translated_Core_Array into UTF-8 file with BOM
if (outfile) {
    WriteToUnicodeFile(full_langpack_array,full_langpack_file);
    WScript.Echo("Langpack file in "+full_langpack_file);
}
if (log) WScript.Echo("Translation end");


//*********************************************************************************//
//                                    Functions                                   *//
//*********************************************************************************//

//output to files. Checking params, and output file(s).
function OutputFiles(TranslatedArray,UntranslatedArray,FileName) {
    //define default path to files in "langpacks\english\plugins"
    TraslatedTemplateFile=trunk+"\\langpacks\\english\\plugins\\translated_"+FileName
    UnTranslatedFile=trunk+"\\langpacks\\english\\plugins\\untranslated_"+FileName
    
    //redefine path to files, if /out specified
    if (out) {
        TraslatedTemplateFile=out+"\\"+FileName;
        UnTranslatedFile=out+"\\untranslated_"+FileName;
        }
        
    //redefine path to files, if FileName is a =CORE=.txt
    if (FileName=="=CORE=.txt") {
        TraslatedTemplateFile=trunk+"\\langpacks\\english\\translated_"+FileName;
        UnTranslatedFile=trunk+"\\langpacks\\english\\untranslated_"+FileName;
        if (out) {
            //if /out:"/path/plugins" specified redefine path of translated and untranslated =CORE=.txt file to parent folder of specified path
            TraslatedTemplateFile=FSO.BuildPath(FSO.GetParentFolderName(out),FileName);
            //if /untranslated:"yes" specified, redefine untranslated core to parent folder, same as above.
            UnTranslatedFile=FSO.BuildPath(FSO.GetParentFolderName(out),"untranslated_"+FileName);
            }
        //if /untralsated:"path/plugins" specified, redefine to parent folder with name "=CORE=.txt"
        if (untranslated && (UnTranslatedPath!="yes")) UnTranslatedFile=FSO.BuildPath(FSO.GetParentFolderName(UnTranslatedPath),FileName);
        }
    
    // output translated file if /out and /outfile ommited, or if /out specified
    if ((!out && !outfile) || out) {
        if (log) WScript.Echo("Output to file:  "+TraslatedTemplateFile);
        WriteToUnicodeFile(TranslatedArray,TraslatedTemplateFile);
        }
    
    //Write untranslated array into file, if /untranslated specified and there is something in array
    if (untranslated & UntranslatedArray.length>0) {
        //redefine Untranslated file path and name, if /untranslated:"/path/plugins" specified and this is not a =CORE=.txt file
        if (UnTranslatedPath!="yes"/* && FileName!="=CORE=.txt"*/) UnTranslatedFile=UnTranslatedPath+"\\"+FileName;
        if (log) WScript.Echo("Untranslated in: "+UnTranslatedFile);
        WriteToUnicodeFile(UntranslatedArray,UnTranslatedFile);
        }
    
    //if we will output one file only, concatenate array
    if (outfile) full_langpack_array=full_langpack_array.concat(TranslatedArray);
     
}

//when /core: and /path: are NOT specified, thus we don't have any langpack file(s) to get translated strings. Thus all other job are useless
function checkparams() { 
if (!WScript.Arguments.Named.Item("langpack") & !WScript.Arguments.Named.Item("path") & !sourcelang)  {
    WScript.Echo("you didn't specify /langpack:\"/path/to/langpack.txt\", /path:\"/path/to/plugnis/\"  or /sourcelang:\"language\" parameter, there is no files with translated strings!");
    WScript.Quit();
    };
}

//Check file existense. If file not found, quit.
function CheckFileExist(file) {
    if (!FSO.FileExists(file)) {
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
if (WScript.Arguments.Named.Item("core")) {
    //Check file /core:"path" exist or not
    CheckFileExist(WScript.Arguments.Named.Item("core"));
    //Generate dictionanry
    GenerateTransalteDict(WScript.Arguments.Named.Item("core"),CoreTranslateDict);
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
 if (sourcelang) GenerateTransalteDict(FSO.BuildPath(translated_plugins,FSO.GetFileName(Template_file)),PluginTranslateDict);
 // if /path:"" specified, this is a folder with plugin translations, use it to find out our translation.
 if (WScript.Arguments.Named.Item("path")) {
    //Generate PluginTranslate Dictionary
    GenerateTransalteDict(FSO.BuildPath(WScript.Arguments.Named.Item("path"),FSO.GetFileName(Template_file)),PluginTranslateDict);
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
     //If we need refference to "; file source\file\path" in template or langpack, put into array every line 
     if (!noref) translated_array.push(line);
     //If we need "clean" file, remove stirngs, starting from ";file"
     if (noref) {
        //RegExp matching strings, starting from ";file"
        reffline=line.match(/^;file.+/);
        //RegExp for match a =CORE=.txt header line "Miranda Language Pack Version 1". If /noref specified, remove this line as well.
        headerline=line.match(/^Miranda Language Pack Version 1$/)
        //if RegExp not matched, push line into array
        if (!reffline && !headerline) translated_array.push(line);
        }
     //find string covered by[] using regexp
     englishstring=line.match(/\[.+\]/);
     //If current line is english string covered by [], try to find translation in global db
     if (englishstring) {
            //uncomment next string for more verbose log output
            //if (log) WScript.Echo("lookin' for "+englishstring);        
            //firstly find our string exist in Plugin translate DB dictionary
            if (PluginTranslateDict.Exists(line)) {
                //yes, we have translation, put translation into array
                translated_array.push(PluginTranslateDict.Item(line));
                } else {
                //If we do not foud sting in plugin translation, check Dupes and if found, put to array
                if (DupesTranslateDict.Exists(line)) {
                    translated_array.push(DupesTranslateDict.Item(line));
                    } else {
                    //not found in dupes? Check CORE
                    if (CoreTranslateDict.Exists(line)) {
                        translated_array.push(CoreTranslateDict.Item(line));
                        } else {
                            //Sill no luck? Check Langpack...
                            if (LangpackTranslateDict.Exists(line)) {
                                translated_array.push(LangpackTranslateDict.Item(line));
                                } else {
                                    //no translation found, put empty line
                                    translated_array.push("");
                                    untranslated_array.push(line);
                                    }
                            }
                    }
                }
            }
    }  
 //closing file
 stream.Close();
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