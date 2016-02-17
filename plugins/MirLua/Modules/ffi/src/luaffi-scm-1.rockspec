package = "luaffi"
version = "scm-1"

source = {
   url = "git://github.com/facebook/luaffifb.git",
}

description = {
   summary = "FFI library for calling C functions from lua",
   detailed = [[
   ]],
   homepage = "https://github.com/facebook/luaffifb",
   license = "BSD"
}

dependencies = {
   "lua >= 5.1",
}

build = {
   type = "builtin",
   modules = {
      ['ffi'] = {
         incdirs = {
            "dynasm"
         },
         sources = {
            "call.c", "ctype.c", "ffi.c", "parser.c",
         }
      },
      ['ffi.libtest'] = 'test.c',
      ['ffi.test'] = 'test.lua',
   }
}
