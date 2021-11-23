
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
#include "tools.h"
typedef string stringtype;
typedef char chartype;
typedef stringstream stringstreamtype;
#include <exexml.h>
#include <exeyaml.h>
#include <exejson.h>
#include <iomanip>
#include <hypefactory.h>


namespace XmlPayload
{
	using namespace XmlFamily;
	struct Item : XmlNode
	{
		friend struct Payload;
		virtual XmlNodeBase* NewNode(Xml& _doc,XmlNodeBase* parent,stringtype name) 
		{ 
			XmlNodeBase* ret(NULL);
			cerr << "\33[33m" << name << "\33[30m" << "|";
			ret=new Item(_doc,parent,name); 
			return ret;
		}
		virtual ostream& operator<<(ostream& o) { XmlNode::operator<<(o); return o;}
		virtual bool operator()(ostream& o) { return XmlNode::operator()(o); }
		Item(Xml& _doc,const XmlNodeBase* _parent,stringtype _name) : XmlNode(_doc,_parent,_name) {}
	};
	inline ostream& operator<<(ostream& o,Item& xmlnode){return xmlnode.operator<<(o);}

	struct Payload : Xml
	{
		enum Modes {Multiply=1,Run=2};
		const Modes mode;
		Payload(const Modes _mode) : mode(_mode) {}
		virtual XmlNode* NewNode(Xml& _doc,stringtype name) { return new Item(_doc,NULL,name); }
		ostream& operator<<(ostream& o) { Xml::operator<<(o); return o;}
		operator Item& () { if (!Root) throw string("No root node"); return static_cast<Item&>(*Root); }
	};
	inline ostream& operator<<(ostream& o,Payload& xml){return xml.operator<<(o);}

} // XmlPayload


void bugjson()
{
	string except;
	try
        {
                stringstream ss;
                ifstream in("ut/oc.json" );
                while ( ! in.eof() )
                {
                        string line; 
                        getline( in, line );
                        ss << line;
                }
		const string j( ss.str() );
                ExeJson::Json json( j );

		if ( ! json ) throw string("Cannot load json");
		const ExeJson::Object& root( json );
		//cout << root << endl;
		vector<string> v
			{ "abc", "int", "txt", "subs", "lst", "real", "yyz" }; 

		{
			for ( vector<string>::iterator sit=v.begin();sit!=v.end();sit++)
			{
				const string name( *sit ); 
				const ExeJson::NodeBase& node( root.GetNode( name ) );
			
				const ExeJson::JsonToken& jc( node );	
				const ExeJson::TokenType& tokentype( jc );	
				if ( tokentype == ExeJson::ObjectOpen )
				{
					for ( vector<string>::iterator sit=v.begin();sit!=v.end();sit++)
					{
						const string subname( *sit ); 
						const ExeJson::NodeBase& subnode( node.GetNode( subname ) );
					
						const ExeJson::JsonToken& subjc( subnode );	
						const ExeJson::TokenType& subtokentype( subjc );	
						if ( subtokentype == ExeJson::ListOpen )
						{
							cout << tab << "List:" << subnode << endl;
							continue;
						}
						const string subv( subnode.vtext() );
						cout << tab << subname << ":" << subv << endl;
					}
					continue;
				}
				if ( tokentype == ExeJson::ListOpen )
				{
					cout << "List:" << node << endl;
					continue;
				}
				const string v( node.vtext() );
				cout << name << ":" << v << endl;
			}
			const ExeJson::NodeBase& nulchk( root.GetNode( "notfound" ) );
			const ExeJson::TokenType& isnul( nulchk );
			if ( isnul == ExeJson::Nothing ) cout << "notfound=NullObject" << endl;
		}
        }
	catch(std::exception& e) {except=e.what();}
	catch(string& s) {except=s;}
	catch(...) {except="Unknown exception";}
	if (except.size()) cerr<<red<<"Exception:" << except<<normal<<endl;
}

bool Diagnose(false);
int main(int argc,char** argv)
{
	string mode,bigs;
        if ( argc < 2 )  { bugjson(); return 0; }


	if (argc>1) mode=argv[1];
	if (argc>2) bigs=argv[2];
	if (mode=="-run") Diagnose=true;
	string except;
	cerr << "Mode:" << mode << endl;
	try
	{
		if (mode=="-multiply")
		{
			int t(0);
			stringstream text;  string b;
			while (!cin.eof()) {getline(cin,b); text<<b<<endl;}
			int howbig(100); if (bigs.size()>1) howbig=atoi(&bigs[1]);	
			cout<<"<multiplicity>"<<endl;
			for (int big=0;big<howbig;big++)
			{
				t++;
				cout<<"\t"<<"<sub "<<endl<<"\t\ttimes=\""<<t<<"\""<<endl<<"\t>"<<endl;	
				string c; c=text.str();
				XmlPayload::Payload config(XmlPayload::Payload::Multiply);
				config.Load(c);
				//config.TabLevel(1);
				XmlFamily::XmlNode& multiples(config);
				multiples(cerr);
				for (XmlFamily::XmlNodeSet::iterator it=multiples.children.begin();it!=multiples.children.end();it++) cout<<(*(*it))<<endl;
				cout<<"\t"<<"</sub>"<<endl;	
			}
			cout<<"</multiplicity>"<<endl;
		}


		if (mode=="-run") 
		{
			XmlPayload::Payload config(XmlPayload::Payload::Run);
			config.Load(cin);
			XmlFamily::XmlNode& multiples(config);
			//config.TabLevel(0);
			multiples(cerr);
			Diagnose=true;
			cout<<multiples<<endl;
		}

		if (mode=="-check") 
		{
			XmlPayload::Payload config(XmlPayload::Payload::Run);
			config.Load(cin);
			//XmlFamily::XmlNode& multiples(config);
			cout<<config<<endl;
			cout << "Check success" << endl;
		}


		if ( mode=="-yamlin" )
		{
			Yaml::stream streaminput( cin );
			HypeFactory::ChainFactory yaml( 2, -1, NULL);
			Yaml::yaml& y( yaml );
			y << streaminput; 
			cout << yaml;
		}

		if ( mode=="-jsonin" )
		{
			stringstream ss;
			while ( ! cin.eof() )
			{
				string line; 
				getline( cin, line );
				ss << line;
			}
			const string j( ss.str() );
			ExeJson::Json json( j );

			if ( ! json ) throw string("Cannot load json");
			const ExeJson::Object& root( json );

			{ ExeJson::JsonOut jout; jout << root; }
		
			cout << endl << root << endl;
		}
	}
	catch(const exception& e) {except=e.what();}
	catch(const string& s) {except=s;}
	catch(...) {except="Unknown exception";}
	if (except.size()) cerr<<red<<"Exception:" << except<<normal<<endl;
	cerr << endl;
	return 0;
}
 
