//*****************************************************************************************//
//* Name:      translate.js                                                               *//
//* Language:  JScript                                                                    *//
//* Function:  Parse Miranda-NG translation templates and get translated strings          *//
//* Author:    BasiL                                                                      *//
//* Usage:     cscript /nologo translate.js  to run generation in batches                 *//
//* Usage:     cscript /nologo translate.js /log:"yes" to enable console logging          *//
//* Usage:     cscript /nologo translate.js /plugin:"path\file" for one template          *//
//* Usage:     cscript /nologo translate.js /path:"path\folder" translate templates folder*//
//* Usage:     cscript /nologo translate.js /core:"path\=core=.txt" use core file         *//
//* Usage:     cscript /nologo translate.js /dupes:"path\=dupes=.txt" use dupes file      *//
//* Usage:     cscript /nologo translate.js /out:"path\folder" output result to folder    *//
//* Usage:     cscript /nologo translate.js /outfile:"path\file" output result to one file*//
//* Usage:     cscript /nologo translate.js /langpack:"path\lang.txt" - Full langpack     *//
//* Usage:     cscript /nologo translate.js /noref:"yes" - remove ref. ";file path\file"  *//
//* Usage:     cscript /nologo translate.js /untranslated:"yes" output untranslated_ files*//
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

// if console param /untranslated: specified, put it to var untranslated. To output untranslated_*  files, specifry /untranslated:"yes"
if (WScript.Arguments.Named.Item("untranslated")) untranslated=true;

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
    //if there is something untranslated in cmdline_untranslated_array, output to file
    if (cmdline_untranslated_array.length>0) WriteToUnicodeFile(cmdline_untranslated_array,untranslated_cmdline_file);
    if (log) WScript.Echo("files here:\r\n"+traslated_cmdline_file+"\r\n"+untranslated_cmdline_file);
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
//var with translated and untranslated core template file path
Translated_Core_File=trunk+"\\langpacks\\english\\translated_=CORE=.txt";
UnTranslated_Core_File=trunk+"\\langpacks\\english\\untranslated_=CORE=.txt";
//if "out" specified, redefine output to this path
if (out) {
    Translated_Core_File=out+"\\=CORE=.txt";
    UnTranslated_Core_File=out+"\\untranslated_=CORE=.txt";
    }
//output traslated core into file, if outfile not specified
if (!outfile) {
    WriteToFile(Translated_Core_Array,Translated_Core_File);
    //if untranslated=true, output to file
    if (untranslated) WriteToFile(UnTranslated_Core_Array,UnTranslated_Core_File);
    }
//loggin output
if (log & !outfile) WScript.Echo("Output to:\r\n"+Translated_Core_File+"\r\n"+UnTranslated_Core_File);
//if output to full langpack, concatenate arrays;
if (outfile) full_langpack_array=full_langpack_array.concat(Translated_Core_Array);

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
     if (log) WScript.Echo("translating "+curfile);
     //path to result files, trunslated and untranslatd
     traslatedtemplatefile=trunk+"\\langpacks\\english\\plugins\\translated_"+FSO.GetFileName(curfile);
     untranslatedfile=trunk+"\\langpacks\\english\\plugins\\untranslated_"+FSO.GetFileName(curfile);
     //if out key specified, output to this folder
     if (out) {
        traslatedtemplatefile=out+"\\"+FSO.GetFileName(curfile);
        untranslatedfile=out+"\\untranslated_"+FSO.GetFileName(curfile);
        }
     //now put strings from template and translations into array
     TranslateTemplateFile(curfile,TranslatedTemplate,UnTranslatedStrings);
     //Write array into file, if outfile not specified
     if (!outfile) {
        WriteToFile(TranslatedTemplate,traslatedtemplatefile);
        //Write untranslated array into file, if /untranslated specified and there is something in array
        if (untranslated & UnTranslatedStrings.length>0) WriteToFile(UnTranslatedStrings,untranslatedfile);
        }
     //if we will output one file only, concatenate array
     if (outfile) full_langpack_array=full_langpack_array.concat(TranslatedTemplate);
     //Log output to console
     if (log & !outfile & !untranslated) WScript.Echo("Output to: "+traslatedtemplatefile);
     if (log & !outfile & untranslated) WScript.Echo("Output to:\r\n"+traslatedtemplatefile+"\r\n"+untranslatedfile);
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

//when /core: and /path: are NOT specified, thus we don't have any langpack file(s) to get translated strings. Thus all other job are uselessg
function checkparams() { 
if (!WScript.Arguments.Named.Item("langpack") && !WScript.Arguments.Named.Item("path"))  {
    WScript.Echo("you didn't specify /langpack: or /path: parameter, there is no files with translated strings!");
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
//path variables
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
var find=/(^\[.+?\])\r\n(.+?)(?=$)/mg;
//While our "find" RegExp return a results, add strings into dictionary.
while ((string = find.exec(translatefiletext)) != null) {
    //first, init empty var
    var string;
    //first match as original string [....], is a key of dictionary, second match is a translation - item of key in dictionary 
    var key=string[1];
    var item=string[2];
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
        //if RegExp not matched, push line into array
        if (!reffline) translated_array.push(line);
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

//Put array of strings into file
function WriteToFile (array,file) {
 //Create file
 out_file=FSO.CreateTextFile(file, overwritefile , unicode)
 //Finally, write strings from array to file
 for (i=0;i<=array.length-1;i++) out_file.WriteLine(array[i]);
 //Close file
 out_file.Close();
};
//Write UTF-8 file
function WriteToUnicodeFile(array,langpack) {
stream.Open();
for (i=0;i<=array.length-1;i++) stream.WriteText(array[i]+"\r\n");
stream.SaveToFile(langpack, 2);
stream.Close();
}