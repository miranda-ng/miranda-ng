git reset --hard

git pull --rebase=true --progress -v 2>&1 > update.log

echo %date% %time% > timestamp.chk