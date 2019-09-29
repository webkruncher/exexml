
#.include <${scripts}/include.mk>


testunit:  testunit.o  $(INCS)
	g++  -D BSD  -frepo -lstdc++  -o testunit testunit.o ${INC} 

testunit.o: testunit.cpp exexml $(INCS)  exexml exeyaml.h exejson
	g++ -D BSD -c -lstdc++ testunit.cpp -o testunit.o ${INC} 

clean:
	-rm *.o
	-rm testunit
	-rm biggy*.*
	-rm benchmark.txt


 
 
 
 
 
