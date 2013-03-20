set version=0.0.0.19
md "setup"
del "setup\winpopup_proto-%version%.zip"
"%ProgramFiles%\WinRar\winrar.exe" a -ep -m5 -cfg- -afzip "setup\winpopup_proto-%version%.zip" winpopup_proto_readme.txt winpopup_proto_translation.txt Win32\release\winpopup_proto.dll
del "setup\winpopup_proto_src-%version%.zip"
"%ProgramFiles%\WinRar\winrar.exe" a -m5 -ed -ep1 -r -cfg- -afzip "setup\winpopup_proto_src-%version%.zip" *.* -x*\.svn\* -x*\debug\* -x*\release\* -x*\setup\* -x*.user -x*.aps -x*.ncb -x*.suo