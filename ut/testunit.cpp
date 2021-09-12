
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <iostream>
#include <fstream>
#include <deque>
using namespace std;
typedef string stringtype;
typedef char chartype;
typedef stringstream stringstreamtype;
#include <exexml.h>
#include <iomanip>


#include <regex.h>


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


bool Diagnose(false);
int main(int argc,char** argv)
{
	string mode,bigs;
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

	}
	catch(std::exception& e) {except=e.what();}
	catch(string& s) {except=s;}
	catch(...) {except="Unknown exception";}
	if (except.size()) cerr<<except<<endl;
	ostream& pout((mode=="-run")?cout:cerr);
	cerr << endl;
	return 0;
}

 
 
 
 
 
