
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <iostream>
#include <fstream>
#include <deque>
#include <regex.h>
using namespace std;
#include <infotools.h>

using namespace KruncherTools;

int main( int argc, char** argv )
{
	string except;
	try
	{
		if ( argc < 3 ) { cout << "Usage: cat file.txt | textslicer starpos endpos " << endl; return -1; }
		stringstream ss;
		while ( ! cin.eof() )
		{
			string s;
			getline( cin, s );
			ss << s << endl;
		}
		const size_t start( stoll( argv[ 1 ] ) );
		const size_t end( stoll( argv[ 2 ] ) );
		if ( start >= end ) throw string("Invalid input");
		if ( end >= ss.str().size() ) throw string("Invalid input");
		
		cout << ss.str().substr( start, end-start ) << endl;
	}
	catch(std::exception& e) {except=e.what();}
	catch(string& s) {except=s;}
	catch(...) {except="Unknown exception";}
	if (except.size()) cerr<<red<<except<<normal<<endl;
	return 0;
}

