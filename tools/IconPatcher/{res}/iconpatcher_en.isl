[LangOptions]
LanguageName=English
LanguageID=$0409
LanguageCodePage=0

[CustomMessages]
; *** "UpdateForm form"
cmWaiting=Waiting...
cmFileNotFound=Archives not found.
cmBinariesPath=Enter path to executable file Miranda32/64.exe:
cmFilter=Miranda??.exe file (Miranda??.exe)|Miranda??.exe
cmSelectArchive=Select an archive from the list:
cmSelectResources=Select resources:
cmSelectResButtonCaption1a=Select all
cmSelectResButtonCaption1b=Nothing
cmSelectResButtonCaption1c=Reset
cmPatchButtonCaption=Patch
cmStatusLabelCaption1a=Processing: %1
cmStatusLabelCaption1b=Ready

; *** "Logs"
cmLogFindResource7ZipFile1a=Resource archive search started
cmLogFindResource7ZipFile1b=Resource archive search finished
cmLogResource7ZipFile=Archive found: %1
cmLogResource7ZipFileContent=List of archive files: %1
cmLogSelectedBinaryFile=Executable file selected: %1
cmLogSelectedResource7ZipFile=Resource file selected: %1
cmLogUpdateResources1a=Resource patching started
cmLogUpdateResources1b=Resource patching finished
cmLogExtractResource7ZipFile=Resource unpacking for %1.* is %2
cmLogUpdateFileResourcesStatus1a=Resource replacement for file %1: succeeded
cmLogUpdateFileResourcesStatus1b=Resource replacement for file %1: failed
cmLogUpdateFileResourcesStatus1c=Resource updating: skipped. None of %1 files found
cmLogResultCode=Return code: %1

; *** "Help Message"
cmHelpMsgCaption=Command-line syntax %1
cmHelpMsg=Usage:%n  %1 [ /? | /bin="exepath" | /arc="arcname" | /res=reslist|* ]%n%nWhere%n  /? - displays this help message%n  exepath - path to Miranda32/64.exe%n  arcname - 7z-archive filename with resources without extension%n  reslist - resource list (or list of root folders), comma separated