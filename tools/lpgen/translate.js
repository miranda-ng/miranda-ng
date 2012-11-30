//**********************************************************************************//
//* Name:         translate.js                                                     *//
//* Language:     JScript                                                          *//
//* Function:     Parse Miranda-NG translation templates and get translated strings*//
//* Usage:        cscript /nologo translate.js  to run generation in batches       *//
//* Usage:        cscript /nologo translate.js /log:"yes" to enable console logging*//
//* Usage:        cscript /nologo translate.js /path:"path\file" for one template  *//
//* Author:       BasiL                                                            *//
//**********************************************************************************//

//Init Variables
//Create FileSystemObject FSO
var FSO=WScript.CreateObject("Scripting.FileSystemObject");
//FileSystemObject vars
var ForReading=1;
var TristateUseDefault=-2;
var overwritefile=true;
var unicode=false;
//disabling log by default
var log="no"
//Path variables
var scriptpath=FSO.GetParentFolderName(WScript.ScriptFullName);
//crazy way to get path two layers upper "\tools\lpgen\"
var trunk=FSO.GetFolder(FSO.GetParentFolderName(FSO.GetParentFolderName(scriptpath)));
//plugins path
var plugins=FSO.BuildPath(trunk,"langpacks\\ru\\Plugins\\");
//init translate db arrays
CoreTranslateDB=new Array;
DupesTranslateDB=new Array;

//*********************************************************************************//
//                         Checking command line parameters                       *//
//*********************************************************************************//

// if console param /log: specified, put it to var log. To enable log, specify /log:"yes"
if (WScript.Arguments.Named.Item("log")) log=WScript.Arguments.Named.Item("log").toLowerCase();

//If script run by double click, open choose folder dialog to choose plugin folder to parse. If Cancel pressed, quit script.
if (WScript.FullName.toLowerCase().charAt(WScript.FullName.length - 11)=="w") {
   WScript.Echo("Please run from command line!");
   WScript.Quit();
}

//when /plugin: specified, parse only this path and quit
if (WScript.Arguments.Named.Item("path")) {
    //First, generate DB of translations from Core and Dupes files
    DupesAndCoreTranslationDB();
    //Path from command line:
    var cmdline_file=new String(WScript.Arguments.Named.Item("path"));
    //init array for our file translation
    var cmdline_file_array=new Array;
    //Output filename variable
    var traslated_cmdline_file=new String(FSO.BuildPath(scriptpath,FSO.GetFileName(cmdline_file)));
    if (log=="yes") WScript.Echo("translating "+cmdline_file);
    //Call TranslateTemplateFile for path specified in command line argument /path:"path/to/template", output result to "scriptpath"
    TranslateTemplateFile(WScript.Arguments.Named.Item("path"),cmdline_file_array);
    //Output result to scriptpath folder.
    WriteToFile(cmdline_file_array,traslated_cmdline_file);
    if (log=="yes") WScript.Echo("translated file here: "+traslated_cmdline_file);
    //We are done, quit.
    WScript.Quit();
}

//*********************************************************************************//
//                                    Main part                                   *//
//*********************************************************************************//

if (log=="yes") WScript.Echo("Translation begin");
//Generate Translation DB as two-dimensional array of Dupes and Core translations.
DupesAndCoreTranslationDB ();

//Array for translated core
Traslate_Core=new Array;
if (log=="yes") WScript.Echo("Translating Core");
//Call function for translate core template
TranslateTemplateFile(FSO.BuildPath(trunk,"langpacks\\en\\=CORE=.txt"),Traslate_Core);
//var with translated core template file path
Translated_Core=trunk+"\\langpacks\\en\\translated_=CORE=.txt"
if (log=="yes") WScript.Echo("Output to: "+Translated_Core);
//output traslated core into file
WriteToFile(Traslate_Core,Translated_Core);

//Init array of template files
TemplateFilesArray=new Array;
//Find all template files and put them to array
FindFiles(FSO.BuildPath(trunk,"langpacks\\en\\plugins\\"),"\\.txt$",TemplateFilesArray)
//create enumerator filesenum from filelist
filesenum=new Enumerator(TemplateFilesArray);
//cycle through file list
 while (!filesenum.atEnd()) {
     //intit Array with translated strings
     TranslatedTemplate=new Array;
     //curfile is our current file in files enumerator
     curfile=filesenum.item();
     //Log output to console
     if (log=="yes") WScript.Echo("translating "+curfile);
     //path to result file
     traslatedtemplatefile=trunk+"\\langpacks\\en\\plugins\\translated_"+FSO.GetFileName(curfile);
     //now put strings from template and translations into array
     TranslateTemplateFile(curfile,TranslatedTemplate);
     //Write array into file;
     WriteToFile(TranslatedTemplate,traslatedtemplatefile);
     //Log output to console
     if (log=="yes") WScript.Echo("Output to: "+traslatedtemplatefile);
     //move to next file
     filesenum.moveNext();
    };
if (log=="yes") WScript.Echo("Translation end");


//*********************************************************************************//
//                                    Functions                                   *//
//*********************************************************************************//

//Generate DB with translations from Core and Dupes files
function DupesAndCoreTranslationDB () {
//path variables
var CoreTranslateFile=FSO.BuildPath(trunk,"langpacks\\ru\\=CORE=.txt");
var DupesTranslateFile=FSO.BuildPath(trunk,"langpacks\\ru\\=DUPES=.txt");
//Generate Core and Dupes translate DBs
GenerateTransalteArray(CoreTranslateFile,CoreTranslateDB);
GenerateTransalteArray(DupesTranslateFile,DupesTranslateDB);
}

//Generate two-dimensional array with english sting + translated string from langpack file
function GenerateTransalteArray (file,array) {
//if file does not exist, it's a core, we do not need do the job again, so return.
if (!FSO.FileExists(file)) return;
//open file
var translatefile=FSO.GetFile(file).OpenAsTextStream(ForReading, TristateUseDefault);
//read file into var
var translatefiletext=translatefile.ReadAll();
//"find" - RegularExpression, first string have to start with [ and end with]. Next string - translation
var find=/(^\[.+?\])\r\n(.+?)(?=\r)/mg;
//While our "find" RegExp return a results, push strings into array.
while ((string = find.exec(translatefiletext)) != null) {
    //first, init empty var
    var string;
    //now, push first match as original string [....], second match is a translation
    array.push([string[1],string[2]]);
    }
//close file 
translatefile.Close();
}

//Generate array with stirngs from translation template, adding founded translation, if exist.
function TranslateTemplateFile(Template_file,array) {
 //init Arrays:
 //1) PluginTranslate from plugins translate file
 PluginTranslateDB=new Array;
 //2) Global translate DB
 GlobalPluginTranslateDB=new Array;
 //Generate PluginTranslate DB
 GenerateTransalteArray(FSO.BuildPath(plugins,FSO.GetFileName(Template_file)),PluginTranslateDB);
 //Concatenate three translate DB in order: Plugin transalte, than Dupes and ending with Core translate DB
 GlobalPluginTranslateDB=PluginTranslateDB.concat(DupesTranslateDB).concat(CoreTranslateDB);
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
            //if (log=="yes") WScript.Echo("lookin for "+englishstring);
            
            //add string  to array, firstly find our string in global translate DB
            //if translation not found (thus, FindTranslation return nothing), empty line will be added.
            array.push(FindTranslation(englishstring,GlobalPluginTranslateDB));
            }
    }  
 //closing file
 template_file_stream.Close();
};

//Cycle trough array and find translation for given string in translate_array
function FindTranslation (string, translate_array) {
//cycle trough array length
for (i=0;i<=translate_array.length-1;i++) {
    //check if the string match our first [0] colomn of array with english strings
    if (translate_array[i][0]==string) {
        //yes, we we have this string, so return second [1] colomn of array
        return translate_array[i][1]
        }
    };
}

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