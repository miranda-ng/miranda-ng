#!/bin/sh
make test || exit
rm -rf test_includes
mkdir -p test_includes
for f in /usr/include/*.h
do
  gcc -E -c "${f}" > tmp 2>/dev/null
  if [ $? == 0 ]
  then
    echo "${f}";
    lua -e '
      local str = io.read("*a")
      -- remove preprocessor commands eg line directives
      str = str:gsub("#[^\n]*", "")
      -- remove inline function definitions and declarations
      str = str:gsub("extern%s+__inline__", "static")
      str = str:gsub("extern%s+__inline", "static")
      str = str:gsub("static[^;(]+__attribute__%s*%b()", "static ")
      str = str:gsub("static[^;(]+__attribute__%s*%b()", "static ")
      str = str:gsub("static[^;(]+%b()%s*%b{}", "")
      str = str:gsub("static[^;(]+%b()%s*;", "")
      io.write(str)' > "test_includes/`basename $f`" < tmp
    lua -e 'ffi = require("ffi"); ffi.cdef(io.read("*a"))' < "test_includes/`basename $f`"
  fi
done
