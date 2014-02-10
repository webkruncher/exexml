
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
#include "exexml"
#include <iomanip>
#define _USE_KRUNCH_PROFILER 
#include <toctic>

namespace XmlPayload
{
	using namespace XmlFamily;
	struct Item : XmlNode
	{
		friend struct Payload;
		virtual XmlNodeBase* NewNode(Xml& _doc,XmlNodeBase* parent,stringtype name) 
		{ 
			XmlNodeBase* ret(NULL);
			tictoc(true,"NewNode","","");
			if (name=="tick") ret=new tick(_doc,parent,name); 
			else ret=new Item(_doc,parent,name); 
			tictoc(false,"NewNode","","");
			return ret;
		}
		virtual ostream& operator<<(ostream& o) { XmlNode::operator<<(o); return o;}
		virtual bool operator()(ostream& o) { return XmlNode::operator()(o); }
		Item(Xml& _doc,const XmlNodeBase* _parent,stringtype _name) : XmlNode(_doc,_parent,_name) {}
	private:

		struct tick : XmlNode, timespec
		{
			virtual XmlNodeBase* NewNode(Xml& _doc,XmlNodeBase* parent,stringtype name) { return new tick(_doc,parent,name);}
			virtual ostream& operator<<(ostream& o) { XmlNode::operator<<(o); return o;}
			virtual bool operator()(ostream&);
			tick(Xml& _doc,const XmlNodeBase* _parent,stringtype _name) : XmlNode(_doc,_parent,_name) {}
		};
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

	bool Item::tick::operator()(ostream& o) 
	{ 
		tictoc(true,"Item::tic()","","");
		Payload& doc(static_cast<Payload&>(Document));
		clock_gettime(CLOCK_MONOTONIC,this);
		stringstream ss; ss<<tv_sec<<"."<<tv_nsec;
		if (doc.mode==Payload::Multiply)
		{
			textsegments[textsegments.size()]=TextElement(Document,this,"");	// cheat. need a couple dummies to help tabs and endls
			textsegments[textsegments.size()]=TextElement(Document,this,"\n");	// tbd: update exexml << to deal with this better
			attributes["when"]=TextElement(Document,this,ss.str());
			XmlNodeBase* newnode(new Item(Document,this,"SomeStuff"));
			if (!newnode) throw string("out of memory");
			appendChild(newnode);
			XmlNode& nn(static_cast<XmlNode&>(*newnode));
			nn.textsegments[nn.textsegments.size()]=TextElement(Document,&nn,"What should I put here?");
			nn.textsegments[nn.textsegments.size()]=TextElement(Document,&nn,"Maybe more samples?");
			nn.textsegments[nn.textsegments.size()]=TextElement(Document,&nn,"How 'bout some more stuff?");
		}
		tictoc(false,"Item::tic()","","");
		return XmlNode::operator()(o); 
	}
} // XmlPayload


bool Diagnose(false);
int main(int argc,char** argv)
{
	tictoc(true,"main","","");
	string mode,bigs;
	if (argc>1) mode=argv[1];
	if (argc>2) bigs=argv[2];
	if (mode=="-run") Diagnose=true;
	string except;
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
				config.TabLevel(1);
				XmlFamily::XmlNode& multiples(config);
				tictoc(true,"multiplicity","","");
				multiples(cerr);
				tictoc(false,"multiplicity","","");
				for (XmlFamily::XmlNodeSet::iterator it=multiples.children.begin();it!=multiples.children.end();it++) cout<<(*(*it))<<endl;
				cout<<"\t"<<"</sub>"<<endl;	
			}
			cout<<"</multiplicity>"<<endl;
		}


		if (mode=="-run") 
		{
			tictoc(true,"Loader","","");
			XmlPayload::Payload config(XmlPayload::Payload::Run);
			config.Load(cin);
			tictoc(false,"Loader","","");
			XmlFamily::XmlNode& multiples(config);
			config.TabLevel(0);
			multiples(cerr);
			Diagnose=true;
			{
				ofstream oo("biggy.check.xml");
				tictoc(true,"Outter","","");
				oo<<multiples<<endl;
				tictoc(false,"Outter","","");
			}
		}

	}
	catch(std::exception& e) {except=e.what();}
	catch(string& s) {except=s;}
	catch(...) {except="Unknown exception";}
	if (except.size()) cerr<<except<<endl;
	tictoc(false,"main","","");
	ostream& pout((mode=="-run")?cout:cerr);
	KrunchProfiler::Profile(pout);
	return 0;
}

 
 
 
 
 
