
rm big*

if [ -z ${1} ]; then
	howmany=3
else
	howmany=${1}
fi



v=$(svn info . | grep Revision | cut -d ' ' -f2)
echo "ExeXml $v" 

if [ ! -f generate ]; then
	make
	if [ ! -f generate ]; then
		echo "generate program was not built successfully."
		exit 0
	fi
fi


echo "multiplying ${howmany} times, starting at $(date)" 
cat myconfig.xml | ./generate -multiply -${howmany} 1> biggy.xml
when=$(date)
ls -lt biggy.xml 
echo "$(cat biggy.xml | wc ) characters" 
echo "${when} regergetating" 

cat biggy.xml | ./generate -run -0 
echo "Started:  ${when}" 
echo "Complete: $(date)" 
echo "diff biggy.xml biggy.check.xml" 
ls -lt biggy.check.xml 
diffs=$(diff biggy.xml biggy.check.xml | wc -l)
echo "diff count: ${diffs}" 
if [ $diffs -ne 0 ]; then
	diff biggy.xml biggy.check.xml 
fi	


