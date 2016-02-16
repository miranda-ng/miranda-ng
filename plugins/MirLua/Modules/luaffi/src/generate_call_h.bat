lua.exe dynasm\dynasm.lua -LNE -D X32WIN -o call_x86.h call_x86.dasc
lua.exe dynasm\dynasm.lua -LNE -D X64 -o call_x64.h call_x86.dasc
lua.exe dynasm\dynasm.lua -LNE -D X64 -D X64WIN -o call_x64win.h call_x86.dasc
lua.exe dynasm\dynasm.lua -LNE -o call_arm.h call_arm.dasc
