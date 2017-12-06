      How to automatically compile Miranda NG installer:


1. Run make_all.bat (will download, compile and cleanup all) or make_interactive.bat for user interactive mode.

2. Collect the compiled installer files from Output folder.

3. Optionally run cleanup.bat if you want to delete all the temporary files and build folders (if make_all.bat didn't do it for you already).


     If you wish to debug or compile script manually in Inno Setup GUI compiler or Inno Script Studio:
	 
	 
1. Run createstructure.bat.

2. Browse to InnoNG_32 or InnoNG_64 folder.

3. Run GenerateInnoGuiCompatScript.bat.

4. Load MirandaNG32.iss or MirandaNG64.iss to Inno Setup GUI compiler or Inno Script Studio (click on the script).

5. Do whatever you wanted to do with it ;)