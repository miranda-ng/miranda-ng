:: This script automatically builds langpack_czech.txt, the Czech translation
:: for Miranda-IM
::
:: $Id: build.bat 5057 2006-02-22 10:10:55Z sindlarv $

@ECHO ON

:: Let's start by adding header
TYPE header.txt > langpack_czech.txt
::
ECHO. >> langpack_czech.txt
ECHO. >> langpack_czech.txt

:: We continue with the translation of Miranda itself
TYPE miranda-translation.txt >> langpack_czech.txt
:: 
ECHO. >> langpack_czech.txt
ECHO. >> langpack_czech.txt

:: Now we are about to add some not yet localized strings
TYPE unpublished-translation.txt >> langpack_czech.txt

ECHO. >> langpack_czech.txt
ECHO. >> langpack_czech.txt
ECHO ; >> langpack_czech.txt
ECHO ; *** PLUGINS ***  >> langpack_czech.txt
ECHO ; >> langpack_czech.txt
ECHO. >> langpack_czech.txt

:: Creates and processes list of *.txt files in the Plugin directory
DIR plugins\*.txt /b /a:-d | SORT /L C > %TEMP%\}{.dat

FOR /F "tokens=*" %%a IN (%TEMP%\}{.dat) DO (
 ECHO. >> langpack_czech.txt
 ECHO. >> langpack_czech.txt
 TYPE "plugins\%%a" >> langpack_czech.txt )

DEL %TEMP%\}{.dat

:: That's all folks !
