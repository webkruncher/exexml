
rm generate
v=$(svn info . | grep Revision | cut -d ' ' -f2)
echo "Revision:$v" > benchmark.txt
./go 10 | tee -a benchmark.txt
./go 100 | tee -a benchmark.txt
./go 500 | tee -a benchmark.txt
./go 2000 | tee -a benchmark.txt
./go 4000 | tee -a benchmark.txt
./go 6000 | tee -a benchmark.txt
