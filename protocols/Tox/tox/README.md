Prerequisites
-------------

You will need PowerShell for preparing the `.def` file.

### Tox-Core

Download Tox DLL from [the CI server](https://jenkins.libtoxcore.so/job/toxcore_win32_dll/). Put that DLL into `dll` directory, copy the `include` files.

Now you need to prepare the `.def` file for the DLL. You have to prepare the corresponding `.lib` file for the DLL. To do it, start PowerShell and enter a Visual Studio command environment. Then execute the following script:

    PS> 'EXPORTS' | Out-File .\libtox.def -Encoding ascii; dumpbin.exe /exports .\libtox.dll | % { ($_ -split '\s+')[4] } | Out-File .\libtox.def -Append -Encoding ascii

It prepares the `def` file for the next step - the `lib` tool:

    PS> lib /def:libtox.def /out:libtox.lib /machine:x86

You'll get the `libtox.lib` import library in the current directory. Copy it to `lib` project directory and you're ready!
