
OStag=`uname -a | cut -d ' ' -f1`
[ ${OStag} == "Linux" ] && export OS=LINUX
[ ${OStag} == "OpenBSD" ] && export OS=UNIX


if [ "${1}" == "rebuild" ]; then
	sudo rm -rf ../src.build
	pushd ~/informationkruncher/src
	sudo rm -rf ../src.build
	./go
	sudo ./go install
	popd
	./go
	sudo pkill webkruncher
	sudo ./go install
	sudo webkruncher ${2-}
fi


echo -ne "\033[32mBuilding WebKruncher for ${OS}\033[0m\n"
mkdir -p ../src.build
cmake  -S . -B ../src.build/  
cmake  --build ../src.build/ 
[ $? != 0 ] && exit -1

[ "${1}" == "install" ] &&  cmake --install ../src.build
exit 0
