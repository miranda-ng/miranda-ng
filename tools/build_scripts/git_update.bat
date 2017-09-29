if "%1" == "" (
   echo Branch should be specified in the first parameter
   exit 1
)

git checkout %1

git reset --hard

git pull --rebase=true --progress -v

echo %date% %time% > timestamp.chk