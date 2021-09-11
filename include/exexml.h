/*
* Copyright (c) Jack M. Thompson WebKruncher.com, exexml.com
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the WebKruncher nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Jack M. Thompson ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Jack M. Thompson BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// JackMThompson.com
/*
	ExeXml Jack M. Thompson WebKruncher.com
	Original creation: circa 2005, major performance improvements and cleanup in 2011
	exexml is designed to read xml into a tight knit C++ hierarchy, and write it back out.
	Every node is based on a generic node, and can be customized.
	The only dependency is C++ / STL.
	The utility does not transform or validate.  
	exexml is designed to be fully contained in a single header file that should never exceed 1500 lines of code.
*/

#ifndef __EXECUTABLE_XML__
#define __EXECUTABLE_XML__
#include <list>

namespace XmlFamilyUtils {class XmlMapNode; class XmlNodeGuts;}

namespace XmlFamily
{
	#define chartype char
	#define stringtype string
	#define stringstreamtype stringstream
	class XmlException : public std::exception
	{
		friend void XmlError(const chartype* a);
		friend void XmlError(const chartype* a,const chartype* b);
		XmlException(){}
		virtual ~XmlException() throw() {}
		XmlException(chartype* a,chartype* b)  { stringstreamtype ss;ss<<a<<" "<<b; msg=ss.str();}
		public: virtual const chartype* what() const throw() {return msg.c_str();}
		private: string msg;
	};

	inline void XmlError(const chartype* a){throw XmlException(const_cast<chartype*>(a),NULL);}
	inline void XmlError(const chartype* a,const chartype* b) {throw XmlException(const_cast<chartype*>(a),const_cast<chartype*>(b));}
	#define whitespaces ((chartype*)" \f\v\r\n\t")

	class XmlNode;class Xml;class XmlNodeBase; class XmlNodeSet; 
	class TextElement; class XmlAttributes; class TextSegments;

	Xml& DeadDoc();
	XmlNode& DeadRoot();
	
	class ElementBase
	{
		friend class Xml;
		friend class XmlFamilyUtils::XmlNodeGuts; 
		friend class XmlNodeBase;
		friend class XmlNodeSet;
		friend class TextElement;
		friend class XmlAttributes;
		friend ostream& operator<<(ostream& o,const ElementBase& e);
		explicit ElementBase(Xml& _doc,const XmlNodeBase* _parent) : parent(_parent),Document(_doc) {}
		virtual ~ElementBase() {}
		const XmlNodeBase* parent;
	public:
		Xml& GetDoc(){return Document;}
		const XmlNodeBase* Parent() const {return (XmlNodeBase*) parent;}
		virtual ostream& operator<<(ostream& o) const = 0;
		virtual void operator()(istream& i,ostream& o) {}
	protected:
		Xml& Document;
	};
	inline ostream& operator<<(ostream& o,const ElementBase& e){return e.operator<<(o);}


	
	class TextElement : public ElementBase, public stringtype
	{
		friend class XmlFamilyUtils::XmlNodeGuts;
		friend class XmlAttributes;
		friend class TextSegments;
	public:
 		TextElement(Xml& _doc,const XmlNodeBase* _parent) : ElementBase(_doc,_parent)  {}
		TextElement(Xml& _doc,const XmlNodeBase* _parent,stringtype s) : 
			ElementBase(_doc,_parent),stringtype(s)  {}
		//TextElement(const TextElement& a) : ElementBase(Document,a.parent)	{parent=a.parent;stringtype::operator=(a.c_str());}
		TextElement& operator=(const TextElement& a)	
			{ if (&a!=this) {parent=a.parent;stringtype::operator=(a.c_str());}return *this; }
		virtual ~TextElement(){clear();}
		//operator stringtype& () {stringtype& s=(*this); return s;}
	protected:
		virtual ostream& operator<<(ostream& o) const
		{
			size_t s(find_first_not_of(whitespaces));
			if (s!=stringtype::npos)
			{
				size_t e(find_last_not_of(whitespaces));
				if (e!=stringtype::npos)
				{
					e++; if (e>size()) XmlError("error handling text element");
					o<<substr(s,e-s);
				} else {
					o<<substr(s,size()-s);
					if (size()-s) o<<endl;
				}
			} else {
				size_t e(find_last_not_of(whitespaces));
				if (e!=stringtype::npos)
				{
					e++; if (e>size()) XmlError("error handling text element");
					o<<substr(0,e);
					if (e) o<<endl;
				} else o<<endl;
			}
			return o;
		}
	};
	
	class TextSegments : public map<int,TextElement >
	{
		friend class XmlFamilyUtils::XmlNodeGuts; 
		friend ostream& operator<<(ostream&,const TextSegments&);
	public:
		TextSegments(Xml& _doc,const XmlNodeBase* _parent) :  parent(_parent), Document(_doc){}
		TextSegments& operator=(const TextSegments& a)
			{if (&a!=this) ((map<int,TextElement>&) (*this)) = a; return *this;}
		virtual ~TextSegments() {clear();}
		TextElement& operator[] (int ndx)
		{
			if (find(ndx)==end()) 
				insert(pair<int,TextElement>(ndx,TextElement(Document,parent)));
			iterator it=find(ndx); if (it==end()) XmlError("out of memory");
			return it->second;
		}
		ostream& operator<<(ostream& o)  const
			{for (const_iterator it=begin();it!=end();it++) o<<it->second;  return o;}
	private:
		const XmlNodeBase* parent;
		Xml& Document;
	};
	inline ostream& operator<<(ostream& o,const TextSegments& t){return t.operator<<(o);}
	
	class XmlAttributes : public map<stringtype,TextElement> 
	{
	public:
		XmlAttributes(Xml& _doc,const XmlNodeBase* _parent) : parent(_parent), Document(_doc){}
		XmlAttributes& operator=(XmlAttributes& a) 
			{if (&a!=this) ((map<stringtype,TextElement>&) (*this)) = a; return *this;}
		virtual ~XmlAttributes(){clear();}
		bool Exists(stringtype what) {return !(find(what)==end());}
		TextElement& operator[](stringtype what)
		{
			if (find(what)==end()) 
				insert(pair<stringtype,TextElement>(what,TextElement(Document,parent)));
			iterator it=find(what); if (it==end()) XmlError("out of memory");
			return it->second;
		}
	private:
		const XmlNodeBase* parent;
		Xml& Document;
	};
	
	class XmlNodeBase : public ElementBase
	{
		friend class XmlFamilyUtils::XmlNodeGuts;
		friend class Xml;
		XmlNodeBase(Xml& _doc,const XmlNodeBase* _p) : ElementBase(_doc,_p) {}
		XmlNodeBase(Xml& _doc) : ElementBase(_doc,NULL) {}
		virtual ~XmlNodeBase(){}
		virtual XmlNodeBase& Copy(const XmlNodeBase& _a) = 0;
	public:
		virtual XmlNodeBase& operator=(const XmlNodeBase& a) {if (&a==this) return *this; clear(); return Copy(a);}
		virtual stringtype& Name() = 0;
		virtual XmlAttributes& Attributes() = 0;
		virtual void clear() = 0;
		virtual int TabLevel() const = 0;
		virtual int SetTabLevel(int) const = 0;
	protected:
		virtual  void appendChild(XmlNodeBase*) = 0;
		virtual ostream& operator<<(ostream& o) const = 0;
		virtual void operator()(istream& ,ostream&) = 0;
		virtual void ParseAttrs(stringtype& txt) = 0;
		virtual void Load(stringtype& text,XmlFamilyUtils::XmlMapNode& mn) = 0;
		virtual XmlFamily::XmlNodeBase* 
			NewNode(Xml& _doc,XmlNodeBase* parent,stringtype name) const = 0;
	};

	class XmlNodeSet : public vector<ElementBase*>
	{
	public:
		XmlNodeSet(bool _bRef = true) : bReference(_bRef) {}
		virtual ~XmlNodeSet() 
			{if (!bReference)  for (iterator it=begin();it!=end();it++)  if (*it) { delete (*it); *it=NULL; } }
		void clear() 
			{if (!bReference)  for (iterator it=begin();it!=end();it++)  if (*it) {delete (*it);  *it=NULL; } vector<ElementBase*>::clear();}
	private:
		bool bReference;// set (default) if referencing nodes, not controlling lifetime
	} ;
}

namespace XmlFamilyUtils
{
	inline void XmlError(const string _a,const string _b)
	{
		const char* a=const_cast<chartype*>(_a.c_str()); 
		const char* b=const_cast<chartype*>(_b.c_str()); 
		XmlFamily::XmlError(a,b);
	}
	inline void XmlError(const string _a)
	{
		const char* a=const_cast<chartype*>(_a.c_str()); 
		const char* b=NULL;
		XmlFamily::XmlError(a,b);
	}


	class XmlMapNode
	{
	public:
		enum TagType
		{
			unknownTagSymbol=0,
			lessThanSymbol,			//		<tagname
			endingLessThanSymbol,		//		</tagname
			greaterThanSymbol,		//		tagname>
			endingGreaterThanSymbol		//		tagname/>
		};

		#define whitespace "\n\r\t "
		#define whitespace_or_slash "\n\r\t /"
		#define whitespace_or_end_tag "\n\r\t >"
		#define endofattribute "\""
		#define whitespace_or_end_tag_or_slash "\n\r\t />"
		#define cdatastart "<![CDATA["
		#define cdataend   "]]>"
		#define headerstarttag "<?"
		#define headerendtag   "?>"
		#define commentstarttag "<!--"
		#define commentendtag "-->"

		XmlMapNode(size_t p) : 
			type(unknownTagSymbol), 
			Mark1(NULL),		// Must always be lessThanSymbol
			Mark2(NULL),		// May be greaterThanSymbol or endingGreaterThanSymbol
			Mark3(NULL),		// May be NULL or endingLessThanSymbol
			Mark4(NULL),		// May be NULL or greaterThanSymbol
			bDone(false),
			pos(p),prev(NULL),next(NULL),name("") {}
		virtual ~XmlMapNode(){}
		TagType type;
		// these pointers are not to be deleted by this class - their lifetimes are managed elsewhere
		XmlMapNode *Mark1,*Mark2,*Mark3,*Mark4;
		bool bDone;
		size_t pos;
		XmlMapNode *prev,*next;
		stringtype name;
		static void Rip
			(stringtype& txt,const char* tokena,const char* tokenb,std::list<stringtype>& dest)
		{
			size_t start,first(stringtype::npos),last(stringtype::npos),end(0);
			while(1)
			{
				start=txt.find(tokena,end);
				if (start==stringtype::npos) break;
				first=start;
				end=txt.find(tokenb,start);
				if (end==stringtype::npos) break;
				end+=strlen(tokenb);
				last=end;
				dest.push_back(txt.substr(start,end-start));
			}
			if (last!=stringtype::npos) txt.erase(first,last-first);
		}
	};

	struct NodeMapBase : public std::list<XmlMapNode> 
	{
		virtual void excavate(stringtype&,size_t,size_t) = 0; 
		virtual void GreaterThan(XmlMapNode&,stringtype&,NodeMapBase&) = 0;
	};

	class NodeMapper : public NodeMapBase
	{
	public:
		virtual ~NodeMapper(){clear();}
		void excavate(stringtype& text,size_t p,size_t pend=0);
		void Excavate(XmlMapNode& node,stringtype& text,NodeMapBase& xmap);
			void Relate(XmlMapNode&,stringtype&,NodeMapBase&);
			void ResolveType(XmlMapNode&,stringtype&,NodeMapBase&);
			void GreaterThan(XmlMapNode&,stringtype&,NodeMapBase&);
			void LessThan(XmlMapNode&,stringtype&,NodeMapBase&);
			void EndingLessThan(XmlMapNode&,stringtype&,NodeMapBase&);
			void EndingGreaterThan(XmlMapNode&,stringtype&,NodeMapBase&);
			bool Eat(XmlMapNode&,stringtype&,NodeMapBase&,XmlMapNode&);
	};


	inline ostream& tab(ostream& o){o<<"\t";return o;}
	
	class XmlNodeGuts : public XmlFamily::XmlNodeBase
	{
		friend class XmlFamily::Xml;
	public:
		XmlNodeGuts(XmlFamily::Xml& _doc,const XmlNodeBase* _parent,stringtype _name) : 
			XmlNodeBase(_doc,_parent),
			__tablevel(0),
			textsegments(_doc,_parent),
			name(_name), 
			children(false),// control lifetimes of children - not references here
			attributes(_doc,_parent)
		{ } 
		virtual ~XmlNodeGuts() {clear();name.clear();textmarkers.clear();}
		XmlNodeGuts& operator=(const XmlNodeGuts& a) 
		{
			if (&a==this) return *this; 
			clear();  
			return (XmlNodeGuts&)Copy((const XmlNodeBase&)a);
		}
		mutable int __tablevel;

		virtual int TabLevel() const {if (Parent()) return (GetParent().TabLevel()+1); else return __tablevel;}
		virtual int SetTabLevel(int tl) const {__tablevel=tl;return tl;}

		XmlFamily::TextSegments textsegments;
		stringtype name;
		XmlFamily::XmlNodeSet children;
		XmlFamily::XmlAttributes attributes;
		virtual stringtype& Name(){return name;}
		virtual XmlFamily::XmlAttributes& Attributes(){return attributes;}
		virtual void Load(stringtype& text)
		{
			if (!text.size()) XmlError("malformed xml - no text");
			if (text.find("<")==stringtype::npos) XmlError("malformed xml - no <");
			if (text.find(">")==stringtype::npos) XmlError("malformed xml - no >");
			if (text.find("/")==stringtype::npos) XmlError("malformed xml - no /");
			XmlFamilyUtils::NodeMapper xmap;
			xmap.excavate(text,0,text.size());
			if (xmap.begin()!=xmap.end()) Load(text,*xmap.begin());
		}
	        virtual void appendChild(XmlNodeBase* n){children.push_back(n);}
		XmlNodeGuts& GetParent() const
		{
			const XmlNodeBase* p=Parent(); 
			if (!p) XmlError("Can't get a reference to a null parent");
			const XmlNodeGuts* gp=static_cast<const XmlNodeGuts*>(p);
			return *const_cast<XmlNodeGuts*>(gp);
		}	
		virtual void clear(){attributes.clear();textsegments.clear();children.clear();name="";}
	private:
		virtual XmlNodeBase& Copy(const XmlNodeBase& _a)
		{
			XmlNodeGuts& a((XmlNodeGuts&)_a);
			name=a.name;
			__tablevel=a.__tablevel;
			attributes=a.attributes;
			textsegments=a.textsegments;
			XmlFamily::XmlNodeSet& ac=(XmlFamily::XmlNodeSet&)a.children;

			for (XmlFamily::XmlNodeSet::iterator it=ac.begin();it!=ac.end();it++)
			{
				XmlNodeBase& that((XmlNodeBase&)**it);
				XmlFamily::XmlNodeBase* c=(XmlFamily::XmlNodeBase*)
					that.NewNode(Document,this,"");				
				if (!c) XmlError("out of memory");
				c->Copy(that);
				appendChild(c);
			}
			return *this;
		}
	protected:
		virtual void tabs(int tl,ostream& o) const { for (int i=0;i<(TabLevel()+tl);i++) o<<tab;}



		virtual void operator()(istream& i,ostream& o) 
		{
			for (XmlFamily::XmlNodeSet::iterator 
				it=children.begin();it!=children.end();it++)
			{
				if (*it) (*it)->operator()(i,o);
			}
		}
		virtual ostream& operator<<(ostream& o) const
		{
			tabs(0,o); o<<"<"<<name.c_str();
			{
				if (attributes.size()) o<<" ";
				for(XmlFamily::XmlAttributes::const_iterator it=attributes.begin();
					it!=attributes.end();it++)
				{
					o<<endl;
					tabs(1,o);
					o<<it->first.c_str()<<"="<<"\""<<it->second.c_str()<<"\"";
				}
				if (attributes.size()) {o<<endl;tabs(0,o);}
			}
			if (children.size())
			{
				int ndx(0);
				o<<">";
				for (XmlFamily::XmlNodeSet::const_iterator 
					it=children.begin();it!=children.end();it++)
				{
					if (textsegments.find(ndx)!=textsegments.end()) o<<textsegments.at(ndx);
					if (*it) (*it)->operator<<(o);
					ndx++;
				}
				while (textsegments.find(ndx)!=textsegments.end()) {o<<textsegments.at(ndx); ndx++;}
				tabs(0,o); 
				o<<"</"<<name.c_str()<<">";
			} else {
				if (textsegments.size())
				{
					o<<">"; o<<textsegments; 
					o<<"</"<<name.c_str()<<">";
				}  else 
					o<<" />";
			}
			return o;
		}
	private:
		class _nodemap_ : public map<int, pair<XmlMapNode*,XmlMapNode*> > // could be anonamous 
		{
			virtual ~_nodemap_(){clear();}
			friend class XmlNodeGuts;
			void add(size_t childndx,XmlMapNode& tm,XmlMapNode& mn)
			{
				if (find(childndx)!=end())  XmlError("textmarker error");
				(*this)[childndx]=pair<XmlMapNode*,XmlMapNode*>(&tm,&mn);
			}
			void generate(const XmlNodeGuts* _parent,stringtype& intext,
				XmlFamily::TextSegments& ts)
			{
				for(iterator it=begin();it!=end();it++)
				{
					XmlMapNode &a(*it->second.first), &b(*it->second.second);
					int aa(a.pos+1),bb(b.pos);
					ts[it->first].assign(intext,aa,bb-aa);
				}
				clear();
			}
		} textmarkers; 
		
		virtual void attrStrip(stringtype& txt)
		{
			size_t m=txt.find_first_not_of(whitespace);
			if (m==stringtype::npos) return;
			if (txt[m]=='\"') txt.erase(m,1);
			size_t eq=txt.find("\""); 
			if (eq!=stringtype::npos) 
				txt.erase(eq,1);
		}

		virtual void ParseAttrs(stringtype& txt)
		{
			size_t m=0;
			while(1)
			{
				m=txt.find_first_not_of(whitespace,m);
				if (m==stringtype::npos) return;
				size_t eq=txt.find("=",m);
				if (eq!=stringtype::npos)
				{
					size_t e=txt.find_first_of(endofattribute,eq+2);
					if (e==stringtype::npos) e=txt.size();
					stringtype a,b;
					a.assign(txt,m,eq-m);
					eq++;
					b.assign(txt,eq,e-eq);
					attrStrip(b);
					attributes[a]=XmlFamily::TextElement(Document,this,b);
					m=e+1;
				} else {
					size_t e=txt.find_first_of(whitespace_or_end_tag,m);
					if (e==stringtype::npos) e=txt.size();
					if (e<=m) XmlError("Invalid xml - can't parse attributes");
					stringtype a;
					a.assign(txt,m,e-m);
					attributes[a]=XmlFamily::TextElement(Document,this);
					m=e+2;
				}
			}
		}
		
		virtual void IdentifyNode(stringtype& intext,XmlFamilyUtils::XmlMapNode& mn)
		{
			name=mn.name.c_str();
			stringtype txt;
			size_t a=mn.pos+mn.name.size()+1;
			if (mn.type==XmlFamilyUtils::XmlMapNode::endingLessThanSymbol) a++;
			size_t b=mn.Mark2->pos;
			if (mn.Mark2->type==XmlFamilyUtils::XmlMapNode::endingGreaterThanSymbol) 
				while(intext[b]!='/') 
			b--;
			txt.assign(intext,a,(b-a));
			ParseAttrs(txt);
		}

		virtual void Load(stringtype& intext,XmlMapNode& mn)
		{
			IdentifyNode(intext,mn);
			if (mn.Mark3)
			{
				if (!mn.Mark2) XmlError("Invalid xml - no end of name tag");
				XmlMapNode* nn=mn.Mark2;
				int depth=0;
				while ( (nn) && (nn!=mn.Mark3) )
				{
					XmlFamilyUtils::XmlMapNode& wn=*nn;
					switch(nn->type)
					{
					case XmlFamilyUtils::XmlMapNode::unknownTagSymbol:
					case XmlFamilyUtils::XmlMapNode::lessThanSymbol:
						if (!depth)
						{
							if (nn&&nn->prev) textmarkers.add
								(children.size(),*nn->prev,*nn);
							XmlNodeBase* nnode = (XmlNodeBase*)
								NewNode(Document,this,wn.name);
							if (!nnode) XmlError("Out of memory");
							appendChild(nnode);
							nnode->Load(intext,wn);
						}
						depth++;
						break;
					case XmlFamilyUtils::XmlMapNode::endingLessThanSymbol:
						depth--;break;
					case XmlFamilyUtils::XmlMapNode::endingGreaterThanSymbol:
						depth--;break;
					case XmlFamilyUtils::XmlMapNode::greaterThanSymbol:
						break;
					}
					if (depth<0) XmlError("Invalid xml - depth < 0");
					nn=nn->next;
				}
				if (nn&&nn->prev) textmarkers.add(children.size(),*nn->prev,*nn);
				textmarkers.generate(this,intext,textsegments);
			}
		}
	};
} //XmlFamilyUtils

namespace XmlFamily
{
	class XmlNode : public XmlFamilyUtils::XmlNodeGuts
	{
		friend class Xml;
		virtual XmlNodeBase* NewNode(Xml& _doc,XmlNodeBase* parent,stringtype name) const
			{return static_cast<XmlNodeBase*>(new XmlNode(_doc,parent,name));}

		friend ostream& operator<<(ostream& o,const XmlNode& xmlnode);
	public:
		virtual operator Xml* () { return NULL;}
		virtual XmlNodeBase* Generate(XmlNodeBase* parent,string _name) 
		{ 
			if (!parent) XmlError((char*)"Cannot produce a node with no parent");
			XmlNodeBase* nn=NewNode(Document,this,_name); 
			if (!nn) XmlError((char*)"Cannot produce a ",_name.c_str());
			appendChild(nn);
			return nn;
		}
		virtual const XmlNode* Root() const
			{if (!Parent()) return this; else return GetParent().Root();}
		XmlNode& GetParent() const
		{
			const XmlNodeBase* _p=Parent(); 
			if (!_p) XmlError("Can't get a reference to a null parent");
			const XmlFamilyUtils::XmlNodeGuts* p=static_cast<const XmlFamilyUtils::XmlNodeGuts*>(_p); 
			const XmlNode* gp=static_cast<const XmlNode*>(p);
			return *const_cast<XmlNode*>(gp);
		}	

		virtual bool operator()() 
		{
			for (XmlFamily::XmlNodeSet::iterator it=children.begin();it!=children.end();it++) 
			{
				XmlNode& n=static_cast<XmlNode&>(*(*it));
				if (!n()) return false;
			}
			return true;
		}

		virtual bool operator()(ostream& o) 
		{
			for (XmlFamily::XmlNodeSet::iterator it=children.begin();it!=children.end();it++) 
			{
				XmlNode& n=static_cast<XmlNode&>(*(*it));
				if (!n(o)) return false;
			}
			return true;
		}

		virtual bool operator()(stringtype& s) 
		{
			for (XmlFamily::XmlNodeSet::iterator it=children.begin();it!=children.end();it++) 
			{
				XmlNode& n=static_cast<XmlNode&>(*(*it));
				if (!n(s)) return false;
			}
			return true;
		}


		virtual bool operator()(int& ndx, stringtype& s) 
		{
			for (XmlFamily::XmlNodeSet::iterator it=children.begin();it!=children.end();it++) 
			{
				XmlNode& n=static_cast<XmlNode&>(*(*it));
				if (!n(ndx,s)) return false;
			}
			return true;
		}

		virtual bool operator()(XmlNode& x) 
		{
			for (XmlFamily::XmlNodeSet::iterator it=children.begin();it!=children.end();it++) 
			{
				XmlNode& n=static_cast<XmlNode&>(*(*it));
				if (!n(x)) return false;
			}
			return true;
		}

		virtual ~XmlNode() {}
		virtual XmlNode& GetNode(string _name)
		{
			for (XmlFamily::XmlNodeSet::iterator it=children.begin();it!=children.end();it++) 
			{
				XmlNode& n=static_cast<XmlNode&>(*(*it));
				if (n.name==_name) return n;
			}
			stringstreamtype ss; ss<<"Cannot find "<<_name<<" in "<<name<<endl;
			XmlError((char*)ss.str().c_str());
			return DeadRoot();
		}
		
		virtual XmlNode* FindNode(string _name)
		{
			for (XmlFamily::XmlNodeSet::iterator it=children.begin();it!=children.end();it++) 
			{
				XmlNode& n=static_cast<XmlNode&>(*(*it));
				if (n.name==_name) return static_cast<XmlNode*>(*it);
			}
			return NULL;
		}

		virtual bool Exists(string _name)
		{
			for (XmlFamily::XmlNodeSet::iterator it=children.begin();it!=children.end();it++) 
			{
				XmlNode& n=static_cast<XmlNode&>(*(*it));
				if (n.name==_name) return true;
			}
			return false;
		}
		virtual XmlNode& RequireNode(string _name)
		{
			if (!Exists(_name)) Generate(this,_name);
			return GetNode(_name);
		}
		virtual void
			operator+=(stringtype line) 
				{ 
					if (line.find_first_not_of(whitespaces)!=stringtype::npos)
						textsegments[textsegments.size()]=
							XmlFamily::TextElement(GetDoc(),this,line); 
				}
		virtual void
			operator=(stringtype line) 
			{ 
				if (line.find_first_not_of(whitespaces)!=stringtype::npos)
					textsegments[0]=XmlFamily::TextElement(GetDoc(),this,line); 
			}
		virtual void operator()(istream& i,ostream& o) 
			{return XmlFamilyUtils::XmlNodeGuts::operator()(i,o);}
	protected:
		virtual ostream& operator<<(ostream& o)  const
			{return XmlFamilyUtils::XmlNodeGuts::operator<<(o);}
		XmlNode(Xml& _doc,const XmlNodeBase* _parent,stringtype _name) : 
			XmlFamilyUtils::XmlNodeGuts(_doc,_parent,_name) {}
	};
	inline ostream& operator<<(ostream& o,const XmlNode& xmlnode){return xmlnode.operator<<(o);}
        inline XmlNode& operator<<(XmlNode& x,stringtype s)
        {
                x.textsegments[x.textsegments.size()]=XmlFamily::TextElement(x.GetDoc(),&x,s);
                return x;
        }
        inline XmlNode& operator<<(XmlNode& x,double n)
        {
		stringstream ss; ss<<n;
                x.textsegments[x.textsegments.size()]=XmlFamily::TextElement(x.GetDoc(),&x,ss.str().c_str());
                return x;
        }

	class Xml 
	{
	public:
		Xml() : Root(NULL) {}
		Xml(istream& in) : Root(NULL) {Load(in);}
		Xml(const XmlFamily::Xml& a) : Root(NULL) {Copy(a);}
		operator XmlNode& () { if (!Root) XmlError("No root node"); return *Root; }
		//void TabLevel(int tl) const { if (!Root) XmlError("No root node"); Root->SetTabLevel(tl);}
		const Xml& operator=(const Xml& a)
		{
			if (&a==this) return *this; 
			if (Root) delete Root; Root=NULL;
			const Xml& ret=Copy(a);
			return ret;
		}

		virtual ~Xml(){if (Root) delete Root; Root=NULL;Headers.clear();Comments.clear();}
		virtual bool operator()() {if (Root) return (*Root)(); return false;}
		virtual void Load(istream& in,XmlFamily::XmlNode* _root=NULL)
		{
			if (_root) Root=_root;
			else 
			{
				if (Root) delete Root;
				Root = NewNode(*this,"root");
			}
			if (!Root) XmlError("out of memory");
			stringtype text; stringstreamtype ss;
			while (!in.eof())
			{
				string line;
				getline(in,line);
				ss<<line<<endl;
			}
			text=ss.str().c_str();
			RipHeaders(text);
			Root->Load(text);
			text.clear();
		}

		virtual void Load(char* txt)
		{
			Root = NewNode(*this,"root");
			if (!Root) XmlError("out of memory");
			stringtype text(txt);
			RipHeaders(text);
			Root->Load(text);
		}

		virtual void Load(stringtype& text,XmlFamily::XmlNode* _root=NULL)
		{
			if (_root) Root=_root;
			else Root = NewNode(*this,"root");
			if (!Root) XmlError("out of memory");
			RipHeaders(text);
			Root->Load(text);
		}
		XmlNode *Root;
		friend ostream& operator<<(ostream& o,const Xml& xml);
        	std::list<stringtype> Headers,Comments;
		virtual XmlNode* NewNode(Xml& _doc,stringtype name) const
		{
			XmlNode* n = new XmlNode(_doc,NULL,name); 
			if (!n) XmlError("Can't create node"); 
			return n;
		}
		virtual const Xml& Copy(const Xml& a)
		{
			if (Root) XmlError("Trying to copy xml over xml");
			Headers=a.Headers;
			if (!a.Root) return *this;
			Root=NewNode(*this,a.Root->name);
			(*Root)=(*a.Root);
			return *this;
		}
	private:
		virtual void RipHeaders(stringtype& text)
		{
			XmlFamilyUtils::XmlMapNode::Rip(text,headerstarttag,headerendtag,Headers);
		}
		
	protected:
		virtual void operator()(istream& i,ostream& o)
		{
			if (Root) 
			{
				(*Root)(i,o); 
			}
		}
		virtual ostream& operator<<(ostream& o) const
		{
			if (Root) 
			{
				for (std::list<string>::const_iterator it=Headers.begin();it!=Headers.end();it++)
					o<<(*it)<<endl;
				o<<(*Root); 
			}
			return o;
		}
	};
	inline ostream& operator<<(ostream& o,const Xml& xml) {return xml.operator<<(o);}
	inline Xml& DeadDoc()
	{
		XmlError("Attempting to create a node with no document");
		static Xml dangerzone_usewithcare_andfixthisup;
		return dangerzone_usewithcare_andfixthisup;
	}
	inline XmlNode& DeadRoot()
	{
		XmlError("Attempting to create a node with no document");
		static Xml dangerzone_usewithcare_andfixthisup;
		return *dangerzone_usewithcare_andfixthisup.Root;
	}
	struct XmlFactory : std::vector<Xml*> { void operator+=(Xml* f){push_back(f);} };

	typedef XmlFamily::Xml xmldoc;
	typedef XmlFamily::XmlNodeBase xmlnodebase;
	typedef XmlFamily::XmlNode xmlnode;
	typedef XmlFamily::XmlNodeSet xmlnodeset;
	typedef std::list<xmlnodebase*>::iterator xmlit;
	typedef std::list<xmlnodebase*>::reverse_iterator xmlrit;
	#define xmlerror XmlFamily::XmlError
} //XmlFamily

namespace XmlFamilyUtils
{


	struct skipper
	{
		skipper(stringtype& _payload,string _startmark,string _endmark) 
			: payload(_payload),startmark(_startmark),endmark(_endmark) {}
		void operator()()
		{
			size_t cd(0);
			do
			{
				cd=payload.find(startmark,cd);
				if ((cd!=stringtype::npos)&&(cd<payload.size())) {startmarkers.push_back(cd);cd++;}
			} while (cd!=stringtype::npos);
			cd=0;
			do
			{
				cd=payload.find(endmark,cd);
				if ((cd!=stringtype::npos)&&(cd<payload.size())) {endmarkers.push_back(cd);cd++;}
			} while (cd!=stringtype::npos);
		}
		size_t operator()(size_t p)
		{
			if (startmarkers.size())
			{ 
				if (startmarkers.size()!=endmarkers.size()) XmlError("Out of sync finding markers");
				if (p>=startmarkers.front()) p=advance();
			}
			return p;
		}
		private:
		bool nextIsStart()
		{
			if (!endmarkers.size()) return false;
			if (!startmarkers.size()) return false;
			return (startmarkers.front()<endmarkers.front()); 
		}
		size_t advance()
		{
			if (endmarkers.front()<=startmarkers.front()) XmlError("End marker came before start marker");
			int r(0);
			size_t p(0);
			do {
				p=endmarkers.front();
				if (nextIsStart())
				{
					r++;
					startmarkers.pop_front();
				} else {
					r--;
					endmarkers.pop_front();
				}
			} while (r);
			return p+endmark.size();
		}
		const stringtype& payload;
		const stringtype startmark;
		const stringtype endmark;
		deque<size_t> startmarkers;
		deque<size_t> endmarkers;
	};


	inline void NodeMapper::excavate(stringtype& text,size_t p,size_t pend)
	{ 
		skipper skipcdata(text,cdatastart,cdataend); skipcdata();
		skipper skipcomments(text,commentstarttag,commentendtag); skipcomments();

		if (!pend) pend=text.size();
		while ( (p<=pend) && (p!=stringtype::npos) )
		{
			while (1)
			{
				p=(text.find_first_of("<>",p));
				if (p==stringtype::npos)  return;
				size_t c(p);
				if (p) p=skipcdata(c);
				if (p==c) if (p) p=skipcomments(c);
				if (p==c) break;
			}
			if (p!=stringtype::npos) 
			{
				push_back(p);
				Excavate(back(),text,(*this));
				p++;
			}
		}
	}

	inline bool NodeMapper::Eat(XmlMapNode& node,stringtype& text,NodeMapBase& xmap,XmlMapNode& mn)
	{
		if (mn.bDone) return true;
		if (mn.type!=XmlMapNode::lessThanSymbol) XmlError("Did not get a < in  matching tag"); 
		if (!(mn.name==node.Mark3->name)) XmlError(node.Mark3->name.c_str(), "Does not match"); 
		node.name=mn.name;
		node.Mark1=&mn;
		node.Mark2=mn.Mark2;
		mn.Mark3=node.Mark3;
		mn.Mark4=node.Mark4;
		node.bDone=true;
		mn.bDone=true;
		node.Mark3->bDone=true;
		node.Mark2->Mark3=node.Mark3;
		return false;
	}


	inline void NodeMapper::GreaterThan(XmlMapNode& node,stringtype& text,NodeMapBase& xmap)
	{
		NodeMapBase::reverse_iterator relatives(xmap.rbegin());
		if (&*relatives!=&node) XmlError("this is impossible");
		relatives++;
		if (relatives==xmap.rend()) XmlError("bad relative ",&text[node.pos]);
		if ( (!(*relatives).bDone) && ((*relatives).type==XmlMapNode::endingLessThanSymbol) )
		{
			if (xmap.begin()==xmap.end()) XmlError("Cannot parse end tag");
			node.Mark4=&node;
			(*relatives).Mark4=&node;
			node.Mark3=&(*relatives);
			bool searching(true);	
			while(searching)
			{
				relatives++; if (relatives==xmap.rend()) XmlError("bad relative ",&text[node.pos]);
				searching=Eat(node,text,xmap,*relatives);
			}
		} else {
			if ( ((*relatives).bDone) || ((*relatives).type!=XmlMapNode::lessThanSymbol) ) XmlError("bad relative ",&text[node.pos]);
			XmlMapNode& mn=*relatives;
			mn.Mark2=&node;
			node.Mark1=&mn;
			node.Mark2=&node;
			node.name=mn.name;
			node.bDone=true;
		} 
	}


	inline void NodeMapper::Relate(XmlMapNode& node,stringtype& text,NodeMapBase& xmap)
	{
		NodeMapBase::reverse_iterator relatives=xmap.rbegin();
		if (&*relatives!=&node) XmlError("invalid call to Excavate(...)");
		relatives++;
		if (!(relatives==xmap.rend())) 
		{
			XmlMapNode& mn=*relatives;
			mn.next=&node;
			node.prev=&mn;
		}
	}

	inline void NodeMapper::ResolveType(XmlMapNode& node,stringtype& text,NodeMapBase& xmap)
	{
		chartype& chr(text[node.pos]);
		if (chr=='<') 
		{
			node.type=XmlFamilyUtils::XmlMapNode::lessThanSymbol;
			size_t nnw = text.find_first_not_of(whitespace,node.pos+1);
			chartype& tc=text[nnw];
			if (tc=='/') node.type=XmlFamilyUtils::XmlMapNode::endingLessThanSymbol;
		}
		if (chr=='>')
		{
			node.type=XmlFamilyUtils::XmlMapNode::greaterThanSymbol;
			if (node.pos>1) if (text[node.pos-1]=='/') node.type=XmlFamilyUtils::XmlMapNode::endingGreaterThanSymbol;
		}
	}

	inline void NodeMapper::LessThan(XmlMapNode& node,stringtype& text,NodeMapBase& xmap)
	{
		if ((node.pos+1)>text.size()) XmlError("Invalid xml - node pos > text size on lessThanSymbol");
		size_t soname(text.find_first_not_of(whitespace,node.pos+1));
		if (soname==stringtype::npos) XmlError("Cannot deduce name for a node ",&text[node.pos]);
		if (!isalnum(text[soname])) XmlError("Cannot deduce name for a node ",&text[node.pos]);
		size_t eoname(text.find_first_of(whitespace_or_end_tag,soname+1));
		node.name.assign(text,soname,eoname-soname);
		node.Mark1=&node;
	}

	inline void NodeMapper::EndingLessThan(XmlMapNode& node,stringtype& text,NodeMapBase& xmap)
	{
		if ((node.pos+1)>text.size()) XmlError("Invalid xml - node pos > text size on endingLessThanSymbol");
		size_t soname(text.find_first_not_of(whitespace_or_slash,node.pos+1));
		if (soname==stringtype::npos) XmlError("Cannot deduce name for a node ",&text[node.pos]);
		if (!isalnum(text[soname])) XmlError("Cannot deduce name for a node ",&text[node.pos]);
		size_t eoname(text.find_first_of(whitespace_or_end_tag,soname+1));
		node.name.assign(text,soname,eoname-soname);
		node.Mark3=&node;
	}

	inline void NodeMapper::EndingGreaterThan(XmlMapNode& node,stringtype& text,NodeMapBase& xmap)
	{
		if (xmap.begin()==xmap.end()) XmlError("Cannot parse end tag");
		NodeMapBase::reverse_iterator relatives=xmap.rbegin();
		if (&*relatives!=&node) XmlError("this is impossible");
		relatives++;
		if (relatives==xmap.rend()) XmlError("bad relative ",&text[node.pos]);
		XmlMapNode& mn(*relatives);
		if (mn.type!=XmlMapNode::lessThanSymbol) XmlError("end tag cannot be matched");
		mn.Mark2=&node;
		node.Mark1=&mn;
		node.Mark2=&node;
		node.name=mn.name;
		node.bDone=true;
		mn.bDone=true;
	}

	inline void NodeMapper::Excavate(XmlMapNode& node,stringtype& text,NodeMapBase& xmap)
	{
		Relate(node,text,xmap);
		ResolveType(node,text,xmap);
		switch(node.type)
		{
			case XmlMapNode::lessThanSymbol:		LessThan(node,text,xmap); break;		//		<
			case XmlMapNode::endingLessThanSymbol:		EndingLessThan(node,text,xmap); break;		//		</
			case XmlMapNode::greaterThanSymbol:		GreaterThan(node,text,xmap); break;		//		>
			case XmlMapNode::endingGreaterThanSymbol:	EndingGreaterThan(node,text,xmap); break; 	//		/>
			default: 					XmlError("unknownTagSymbol");
		}
	}
} // XmlFamilyUtils


#endif //__EXECUTABLE_XML__
// ExeXml.com / WebKruncher.com / JackMThompson.com

