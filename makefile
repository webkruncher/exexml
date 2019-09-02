
#.include <${scripts}/include.mk>


generate:  generate.o  $(INCS)
	g++  -D BSD  -frepo -lstdc++  -o generate generate.o ${INC} 

generate.o: testunit.cpp exexml $(INCS)  exexml exeyaml.h exejson
	g++ -D BSD -c -lstdc++ testunit.cpp -o generate.o ${INC} 

clean:
	-rm *.o
	-rm generate
	-rm biggy*.*
	-rm benchmark.txt


 
 
 
 
 
