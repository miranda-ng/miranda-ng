@echo off

if not exist .\..\..\bin10\Release\Plugins\langs (
  mkdir .\..\..\bin10\Release\Plugins\langs
)

if not exist .\..\..\bin10\Release64\Plugins\langs (
  mkdir .\..\..\bin10\Release64\Plugins\langs
)

copy /Y langs\Russian.xml .\..\..\bin10\Release\Plugins\langs
copy /Y langs\English.xml .\..\..\bin10\Release\Plugins\langs
copy /Y langs\Russian.xml .\..\..\bin10\Release64\Plugins\langs
copy /Y langs\English.xml .\..\..\bin10\Release64\Plugins\langs
