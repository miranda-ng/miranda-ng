Set objShell = CreateObject("WScript.Shell")
Set objWmi = GetObject("winmgmts:")


strWmiq = "select * from Win32_Process where name='miranda32.exe'"
Set objQResult = objWmi.Execquery(strWmiq)


For Each objProcess In objQResult
intRet = objProcess.Terminate(1)
Next 