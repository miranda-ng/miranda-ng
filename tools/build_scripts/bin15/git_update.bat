pushd ..

git pull --rebase=true --progress -v 2>&1 > update.log

popd