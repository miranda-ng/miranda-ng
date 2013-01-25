//*************************************************************************************//
//* Name:      translate.js                                                           *//
//* Language:  JScript                                                                *//
//* Function:  Parse Miranda-NG translation templates and get translated strings      *//
//* Author:       BasiL                                                               *//
//* Usage:     cscript /nologo translate.js  to run generation in batches             *//
//* Usage:     cscript /nologo translate.js /log:"yes" to enable console logging      *//
//* Usage:     cscript /nologo translate.js /plugin:"path\file" for one template      *//
//* Usage:     cscript /nologo translate.js /path:"path\folder" - translate folder    *//
//* Usage:     cscript /nologo translate.js /core:"path\=core=.txt" use core file     *//
//* Usage:     cscript /nologo translate.js /dupes:"path\=dupes=.txt" use dupes file  *//
//* Usage:     cscript /nologo translate.js /out:"path\folder" output result to folder*//
//* Example1:  cscript /nologo translate.js /core:"path\lang.txt" /path:"path/german" *//
//* will translate english templates using plugins translation from path\german and if*//
//* translation not found, try to find translation in path\lang.txt                   *//
//* Example2:  cscript /nologo translate.js /plugin:"path\file" /core:"path\lang.txt" *//
//* will translate path\file using translation from path\lang.txt                     *//
//*************************************************************************************//

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
//Path variables
var scriptpath=FSO.GetParentFolderName(WScript.ScriptFullName);
//crazy way to get path two layers upper "\tools\lpgen\"
var trunk=FSO.GetFolder(FSO.GetParentFolderName(FSO.GetParentFolderName(scriptpath)));
//plugins path
//var plugins=FSO.BuildPath(trunk,"langpacks\\russian\\Plugins\\");
//init translate dictionaries
CoreTranslateDict=WScript.CreateObject("Scripting.Dictionary");
DupesTranslateDict=WScript.CreateObject("Scripting.Dictionary");

//*********************************************************************************//
//                         Checking command line parameters                       *//
//*********************************************************************************//

// if console param /log: specified, put it to var log. To enable log, specify /log:"yes"
if (WScript.Arguments.Named.Item("log")) log=true;

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
    DupesAndCoreTranslation();
    //plugin from command line:
    var cmdline_file=new String(WScript.Arguments.Named.Item("plugin"));
    //init array for our file translation
    var cmdline_file_array=new Array;
    //Output filename variable
    var traslated_cmdline_file=new String(FSO.BuildPath(scriptpath,FSO.GetFileName(cmdline_file)));
    if (log) WScript.Echo("translating "+cmdline_file);
    //Call TranslateTemplateFile for path specified in command line argument /path:"path/to/template", output result to "scriptpath"
    TranslateTemplateFile(WScript.Arguments.Named.Item("plugin"),cmdline_file_array);
    //Output result to scriptpath folder.
    WriteToFile(cmdline_file_array,traslated_cmdline_file);
    if (log) WScript.Echo("translated file here: "+traslated_cmdline_file);
    //We are done, quit.
    WScript.Quit();
}


//*********************************************************************************//
//                                    Main part                                   *//
//*********************************************************************************//
//first, check we have files with translated stirngs specified.
checkparams();
if (log) WScript.Echo("Translation begin");
//Generate Translation DB as two-dimensional array of Dupes and Core translations.
DupesAndCoreTranslation ();

//Array for translated core
Translate_Core=new Array;
if (log) WScript.Echo("Translating Core");
//Call function for translate core template
TranslateTemplateFile(FSO.BuildPath(trunk,"langpacks\\english\\=CORE=.txt"),Translate_Core);
//var with translated core template file path
Translated_Core=trunk+"\\langpacks\\english\\translated_=CORE=.txt";
//if "out" specified, redefine output to this path
if (out) Translated_Core=out+"\\=CORE=.txt";
//output traslated core into file
WriteToFile(Translate_Core,Translated_Core);
if (log) WScript.Echo("Output to: "+Translated_Core);

//Init array of template files
TemplateFilesArray=new Array;
//Find all template files and put them to array
FindFiles(FSO.BuildPath(trunk,"langpacks\\english\\plugins\\"),"\\.txt$",TemplateFilesArray)
//create enumerator filesenum from filelist
filesenum=new Enumerator(TemplateFilesArray);
//cycle through file list
 while (!filesenum.atEnd()) {
     //intit Array with translated strings
     TranslatedTemplate=new Array;
     //curfile is our current file in files enumerator
     curfile=filesenum.item();
     //Log output to console
     if (log) WScript.Echo("translating "+curfile);
     //path to result file
     traslatedtemplatefile=trunk+"\\langpacks\\english\\plugins\\translated_"+FSO.GetFileName(curfile);
	 //if out key specified, output to this folder
	 if (out) traslatedtemplatefile=out+"\\"+FSO.GetFileName(curfile);
     //now put strings from template and translations into array
     TranslateTemplateFile(curfile,TranslatedTemplate);
     //Write array into file;
     WriteToFile(TranslatedTemplate,traslatedtemplatefile);
     //Log output to console
     if (log) WScript.Echo("Output to: "+traslatedtemplatefile);
     //move to next file
     filesenum.moveNext();
    };
if (log) WScript.Echo("Translation end");


//*********************************************************************************//
//                                    Functions                                   *//
//*********************************************************************************//

//when /core: and /path: are NOT specified, thus we don't have any langpack file(s) to get translated strings. Thus all other job are uselessg
function checkparams() { 
if (!WScript.Arguments.Named.Item("core") && !WScript.Arguments.Named.Item("path"))  {
    WScript.Echo("you didn't specify /core: or /path: parameter, there is no files with translated strings!");
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
function DupesAndCoreTranslation () {
//path variables
if (WScript.Arguments.Named.Item("core")) {
    CheckFileExist(WScript.Arguments.Named.Item("core"));
    //var CoreTranslateFile=FSO.BuildPath(trunk,"langpacks\\russian\\=CORE=.txt");
    GenerateTransalteDict(WScript.Arguments.Named.Item("core"),CoreTranslateDict);
    }
if (WScript.Arguments.Named.Item("dupes")) {
    CheckFileExist(WScript.Arguments.Named.Item("dupes"));
    //var DupesTranslateFile=FSO.BuildPath(trunk,"langpacks\\russian\\=DUPES=.txt");
    GenerateTransalteDict(WScript.Arguments.Named.Item("dupes"),DupesTranslateDict);
    }
}

//Generate Dictionary with english sting + translated string from file
function GenerateTransalteDict (file,dictionary) {
//if file does not exist, it's a core, we do not need do the job again, so return.
if (!FSO.FileExists(file)) return;
//open file
var translatefile=FSO.GetFile(file).OpenAsTextStream(ForReading, TristateUseDefault);
//read file into var
var translatefiletext=translatefile.ReadAll();
//"find" - RegularExpression, first string have to start with [ and end with]. Next string - translation
var find=/(^\[.+?\])\r\n(.+?)(?=\r)/mg;
//While our "find" RegExp return a results, add strings into dictionary.
while ((string = find.exec(translatefiletext)) != null) {
    //first, init empty var
    var string;
    //first match as original string [....], is a key of dictionary, second match is a translation - item of key in dictionary 
    var key=string[1];
    var item=string[2];
    //add key-item pair into dictionary
    dictionary.Add(key,item)
    }
//close file
translatefile.Close();
}

//Generate array with stirngs from translation template, adding founded translation, if exist.
function TranslateTemplateFile(Template_file,array) {
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
 template_file_stream=FSO.GetFile(Template_file).OpenAsTextStream(ForReading, TristateUseDefault);
 //Reading line-by-line
 while (!template_file_stream.AtEndOfStream) {
     //clear up variable
     englishstring="";
     //read on line into rcline
     line=template_file_stream.ReadLine();
     //Push line int array, we need all lines from template
     array.push(line);
     //find string covered by[] using regexp
     englishstring=line.match(/\[.+\]/);
     //If current line is english string covered by [], try to find translation in global db
     if (englishstring) {
            //uncomment next string for more verbose log output
            //if (log) WScript.Echo("lookin' for "+englishstring);        
            //firstly find our string exist in Plugin translate DB dictionary
            if (PluginTranslateDict.Exists(line)) {
                //yes, we have translation, put translation into array
                array.push(PluginTranslateDict.Item(line));
                } else {
                //If we do not foud sting in plugin translation, check Dupes and if found, put to array
                if (DupesTranslateDict.Exists(line)) {
                    array.push(DupesTranslateDict.Item(line));
                    } else {
                    //not found in dupes? Check CORE
                    if (CoreTranslateDict.Exists(line)) {
                        array.push(CoreTranslateDict.Item(line));
                        } else {
                            //no translation found, put empty line
                            array.push("");
                            }
                    }
                }
            }
    }  
 //closing file
 template_file_stream.Close();
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
 outfile=FSO.CreateTextFile(file, overwritefile , unicode)
 //Finally, write strings from array to file
 for (i=0;i<=array.length-1;i++) outfile.WriteLine(array[i]);
 //Close file
 outfile.Close();
};