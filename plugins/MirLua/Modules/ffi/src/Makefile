#
# Use luarocks to install LuaFFI:
# > git clone https://github.com/facebook/luaffifb
# > cd luaffifb && luarocks make
#
# To rebuild the call_* headers:
# > rm call_*.h && make headers
#

.PHONY: build clean headers
LUA=lua

build:
	luarocks make

clean:
	rm -f *.o *.so *.dylib

headers:
	$(MAKE) call_x86.h call_x64.h call_x64win.h call_arm.h

call_x86.h: call_x86.dasc dynasm/*.lua
	$(LUA) dynasm/dynasm.lua -LN -o $@ $<

call_x64.h: call_x86.dasc dynasm/*.lua
	$(LUA) dynasm/dynasm.lua -D X64 -LN -o $@ $<

call_x64win.h: call_x86.dasc dynasm/*.lua
	$(LUA) dynasm/dynasm.lua -D X64 -D X64WIN -LN -o $@ $<

call_arm.h: call_arm.dasc dynasm/*.lua
	$(LUA) dynasm/dynasm.lua -LNE -o $@ $<
