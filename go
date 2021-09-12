
OStag=`uname -a | cut -d ' ' -f1`
[ ${OStag} == "Linux" ] && export OS=LINUX
[ ${OStag} == "OpenBSD" ] && export OS=UNIX


if [ "${1}" == "rebuild" ]; then
	sudo rm -rf ../src.build
	pushd ~/Info/informationkruncher/src
	sudo rm -rf ../src.build
	sudo ./go install
	popd
	./go
	sudo ./go install
fi


echo -ne "\033[32mBuilding ExeXml for ${OS}\033[0m\n"
mkdir -p ../src.build
cmake  -S . -B ../src.build/  
cmake  --build ../src.build/ 
[ $? != 0 ] && exit -1

if [ "${1}" == "-test" ]; then
	shift
	cat ut/myconfig.xml | ../src.build/ut/exeut ${1}
fi


[ "${1}" == "install" ] &&  cmake --install ../src.build
exit 0
