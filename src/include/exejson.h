/*
 * Copyright (c) Jack M. Thompson WebKruncher.com
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
 */

#ifndef BUILDER_JSON_H
#define BUILDER_JSON_H
#include <map>
#include <queue>
#include <infotools.h>
using namespace KruncherTools;

struct CBug : ofstream
{
	CBug() : ofstream( "/dev/stderr" ) {}
};

CBug trace;


namespace ExeJson
{
	enum TokenType { 
		Root=100, 
		ObjectOpen, ObjectClose, ListOpen, ListClose,
		Coma, Coln, NameQuots, ValueQuots, Special, Character,
		ValueChar, Nothing
	};


	inline string GlyphType( const TokenType& tokentype )
	{
		switch ( tokentype )
		{
			case Root       :	return "Root"; break;
			case ObjectOpen :	return "ObjectOpen"; break;
			case ObjectClose:	return "ObjectClose"; break;
			case ListOpen   :	return "ListOpen"; break;
			case ListClose  :	return "ListClose"; break;
			case Coma       :	return "Coma"; break;
			case Coln       :	return "Coln"; break;
			case NameQuots  :	return "NameQuots"; break;
			case ValueQuots :	return "ValueQuots"; break;
			case Special    :	return "Special"; break;
			case Character  :	return "Character"; break;
			case ValueChar  :	return "ValueChar"; break;
			case Nothing   :	return "Nothing"; break;
			default: return "NotAType";
		}
		
	}

	struct GlyphDisposition
	{
		GlyphDisposition() : Enquoted( false ) {}
		bool enquoted( bool toggle=true )
		{
			if ( toggle )
			{
				const bool ret( Enquoted );
				Enquoted=!Enquoted;
				return ret;
			} else {
				return Enquoted;
			}
		}
		private:
		bool Enquoted;
	};

	struct Markers : pair<size_t,size_t>
	{
		typedef pair<size_t,size_t> mtype;
		Markers( const size_t _first, const size_t _second ) : mtype( _first, _second ) {}
		void swap() { const size_t s( second ); second=first; first=s;}
		void operator--(){first--;second--;}
		private:
		friend ostream& operator<<(ostream&,const Markers&);
		ostream& operator<<(ostream& o) const
		{
			if ( second )
				o << "(" << first << ":" << second << ")";
			else o << green << whitebk << "(" << first << ")" << normal; 
			return o;
		}
	}; 

	inline ostream& operator<<(ostream& o,const Markers& m) { return m.operator<<(o); }


	struct QueString;
	struct NodeBase;
	struct JsonToken
	{
		JsonToken( ) : pos( 0, 0 ), tokentype( Root ), c( 0 ) {}
		JsonToken( TokenType _tokentype) : pos( 0, 0 ), tokentype( _tokentype ), c( 0 ) {}
		JsonToken( const size_t _pos, TokenType _t, char _c ) : pos( _pos, 0 ), tokentype( _t ), c( _c ) {}
		JsonToken( const size_t zero, const size_t _pos, TokenType _t, char _c ) : pos( zero, _pos ), tokentype( _t ), c( _c ) {}
		JsonToken( const JsonToken& that ) : pos( that.pos ), tokentype( that.tokentype ), c( that.c ) { }
		const JsonToken& operator=( const JsonToken& that ) const
		{
			if ( this != &that )
			{
				pos=that.pos;
				tokentype=that.tokentype;
				c=that.c;
			}
			return *this;
		}

		operator const char& ()  const { return c; }

		void morph( const TokenType _t, const char _c ) const
		{
			c=_c;
			tokentype=_t;
		}
		operator const TokenType () const { return tokentype; }
		operator Markers& () const { return pos; }
		void swap() const { const size_t s( pos.second ); pos.second=pos.first; pos.first=s;}
		void closure( const Markers& _pos ) const { pos.second=_pos.second-1; }
		operator string () const
		{
			stringstream ss;
			switch ( tokentype )
			{
				case Coma: ss << green << c << normal; break;
				case Coln: ss << bluebk << white << bold << c << normal; break;
				case NameQuots: ss << rvid << yellow << c << normal; break;
				case ValueQuots: ss << rvid << blink << yellow << c << normal; break;
				case ListOpen: ss << black << whitebk << c << normal; break;
				case ListClose: ss << black << whitebk << c << normal; break;
				case ObjectOpen: ss << rvid << bold << c << normal; break;
				case ObjectClose: ss << rvid << ulin << c << normal; break;
				case Special: ss << red << c << normal; break;
				case Character: ss << teal << c << normal; break;
				case Root: ss << bluebk << green << bold << c << normal; break;
				case ValueChar: ss << redbk << yellow << bold << c << normal; break;
				case Nothing: ss << redbk << green << bold << c << normal; break;
				default: ss << tealbk << blue << c;
			}
			return ss.str();
		}
		private:
		friend struct NodeBase;
		mutable Markers pos;
		mutable TokenType tokentype;
		mutable char c;
		friend ostream& operator<<( ostream&, const JsonToken& ); 
		ostream& operator<<( ostream& o ) const 
		{
			o << c;
			return o;
		}
	};

	inline ostream& operator<<( ostream& o, const JsonToken& j ) { return j.operator<<(o); }

	inline string Print( const string& jtxt, const JsonToken& jc )
	{
		stringstream ss;
		const Markers& pos( jc );
		const string& v( Slice( jtxt, pos ) );
		ss << normal << mgenta << GlyphType( jc ) << fence << rvid << jc << pos << fence << v << normal;
		return ss.str();
	}

	struct QueString : queue< JsonToken >
	{
		QueString() = delete;
		QueString( const int _much, GlyphDisposition& _glyphs ) 
			: much( _much ), glyphs( _glyphs ), quotestate( true ) {}
		void operator()( const char c )
		{
			much++;
			const char b4( empty() ? '\0' : back() );
			if ( b4 == '\\' )
			{
				back().morph( Special, c );
			} else {
				switch ( c )
				{
					case '"': 
					{
						const TokenType tt ( ( quotestate ) ? NameQuots : ValueQuots );
						glyphs.enquoted();
						JsonToken jc( much, tt, c ); 
						push( jc ); 
						break;
					}
					case ',': 
					{
						if ( glyphs.enquoted( false ) )
						{
							JsonToken jc( much, Character, c ); 
							push( jc ); 
						} else {
							quotestate=true;  
							JsonToken jc( much, Coma, c ); 
							push( jc ); 
						}
						break; 
					}
				
					case ':': { quotestate=false; JsonToken jc( much, Coln, c ); push( jc ); break; }
					case '{': { quotestate=true;  JsonToken jc( much, ObjectOpen, c ); push( jc ); break; }
					case '}': { quotestate=false; JsonToken jc( 0, much, ObjectClose, c ); push( jc ); break; }
					case '[': { quotestate=true;  JsonToken jc( much, ListOpen, c ); push( jc ); break; }
					case ']': { quotestate=false; JsonToken jc( 0, much, ListClose, c ); push( jc ); break; }
					default: 
					{
						if ( ! glyphs.enquoted( false ) && ( ! WhiteSpace( c )  ) )
						{
							JsonToken jc( much, ValueChar, c ); 
							push( jc ); 
						} else {
							JsonToken jc( much, Character, c ); 
							push( jc ); 
						}
						break;
					}
				}
			}	
		}
		bool enquoted( const bool toggle=true ) { return glyphs.enquoted( toggle ); }
		private:
		bool WhiteSpace( const char cc )
		{
			if ( cc == ' ' ) return true;
			if ( cc == '\t' ) return true;
			if ( cc == '\r' ) return true;
			if ( cc == '\n' ) return true;
			return false;
		}
		int much;
		bool quotestate;
		GlyphDisposition& glyphs;
	};


	struct Excavator;
	struct Object;
	struct NodeBase : vector< NodeBase* >
	{
		friend struct Excavator;
		NodeBase( const string& _jtxt) : jtxt( _jtxt ) {}
		NodeBase( const string& _jtxt, const JsonToken _jc ) : jtxt( _jtxt ), jc( _jc ) {}
		void operator = ( const size_t _endmarker ) { jc.pos.second=_endmarker; }
		virtual ~NodeBase() { for ( iterator it=begin();it!=end();it++) delete *it; }
		virtual bool operator()( const string&, QueString&, const JsonToken& );
		void closure( const Markers& pos ) const { jc.closure( pos ); }
		operator Markers () const { return jc; }
		operator const TokenType () const { return jc; }
		virtual operator const bool () = 0;
		virtual operator string () const = 0;
		virtual operator const Object* () const { return nullptr; }
		operator JsonToken& () const { return jc; }
		virtual const string vtext () const { return ""; }

		virtual const NodeBase& GetNode( const string& name ) const { throw name; }
		protected:
		const string& jtxt;

		mutable JsonToken jc;
		friend ostream& operator<<(ostream&, const NodeBase&);
		virtual ostream& operator<<(ostream& o) const = 0;
		friend CBug& operator<<(CBug&, const NodeBase&);
		virtual CBug& operator<<(CBug& o) const = 0;
	};
	inline ostream& operator<<(ostream& o, const NodeBase& n ) { return n.operator<<(o); }
	inline CBug& operator<<(CBug& o, const NodeBase& n ) { return n.operator<<(o); }

	struct Index : map< string, NodeBase* >
	{
		private:
		friend ostream& operator<<(ostream&, const Index&);
		virtual ostream& operator<<(ostream& o) const ;
	};
	inline ostream& operator<<(ostream& o, const Index& n ) { return n.operator<<(o); }

	struct Node : NodeBase
	{
		Node( const string& _jtxt) : NodeBase( _jtxt) {}
		Node( const string& _jtxt, TokenType _tokentype) : NodeBase( _jtxt, _tokentype ) {}
		Node( const string& _jtxt, const JsonToken _jc ) : NodeBase( _jtxt, _jc ) {}
		virtual operator const bool () 
		{
			string current;
			for ( iterator it=begin();it!=end();it++)
			{
				NodeBase& n( **it );
				const JsonToken& subjc( n );
				const TokenType& subtokentype( subjc );
				const Markers& submarkers( subjc );

				if ( subtokentype == NameQuots ) 
				{
					if ( submarkers.second )
					{
						const Markers& pos( subjc );
						const string& name( Slice( jtxt, pos ) );
						if ( !name.empty() )
						{
							current=name;
						}
					}
				} 
				if ( ! current.empty() ) 
				{
						const string& value( n.vtext() );
						if ( ! value.empty() )
							index[ current ] = &n;
				}


				if ( ! n ) return false;
			}
			return true;
		}
		virtual operator string () const { return ""; }


		protected:
		Index index;
		private:
		virtual ostream& operator<<(ostream& o) const 
		{

			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}
		virtual CBug& operator<<(CBug& o) const = 0;
	};

	
	struct RootNode : Node
	{
		RootNode( const string& _jtxt ) : Node( _jtxt) {}
		operator const Object* () const 
		{
			if ( size() != 1 ) return nullptr;
			const NodeBase* np( *begin() );
			if ( ! np ) throw string("No node");
			const NodeBase& n( *np );
			
			const Object* oo( n );
			return oo;
		}
		private:
		virtual CBug& operator<<(CBug& o) const 
		{
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}
	};


	struct NullObject : Node
	{
		NullObject() = delete;
		NullObject( const string& _jtxt ) : Node( _jtxt, Nothing ) {}
		virtual ostream& operator<<(ostream& o) const { return o; }
		virtual CBug& operator<<(CBug& o) const  { return o; }
	};

	struct Object : Node
	{
		Object( const string& _jtxt, const JsonToken _jc ) 
			: Node( _jtxt, _jc ), nullobject( _jtxt ) {}
		operator const Object* () const { return this; }
		const NodeBase& GetNode( const string& name ) const
		{
			Index::const_iterator it( index.find( name ) );
			if ( it == index.end() ) return nullobject;
			return *it->second;
		}
		virtual const string vtext () const { return "OBJECT"; }
		private:
		NullObject nullobject;
		virtual CBug& operator<<(CBug& o) const 
		{
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}

		virtual ostream& operator<<(ostream& o) const 
		{
			o << index;
			return o;
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}

	};

	struct List : Node
	{
		List( const string& _jtxt, const JsonToken _jc ) : Node( _jtxt, _jc ) {}
		virtual const string vtext () const { return "LIST"; }
		private:
		virtual CBug& operator<<(CBug& o) const 
		{
			o << ulin << jc << normal;
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}

		virtual ostream& operator<<(ostream& o) const 
		{
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				stringstream ss;
				ss << n;
				if ( ss.str().empty() ) continue;
				if ( ss.str().find_first_not_of(" \t\r\n") == string::npos) continue;
				if ( it != begin() ) o << ", ";
				o << ss.str();
			}
			return o;
		}

	};


	struct Comma : Node
	{
		Comma( const string& _jtxt, const JsonToken _jc ) : Node( _jtxt, _jc ) {}
		private:
		virtual CBug& operator<<(CBug& o) const 
		{
			//o << blink << "," << normal;
			return o;
		}

		virtual ostream& operator<<(ostream& o) const 
		{
			return o;
		}
	};

	struct Colon : Node
	{
		Colon( const string& _jtxt, const JsonToken _jc ) : Node( _jtxt, _jc ) {}
		private:
		virtual CBug& operator<<(CBug& o) const 
		{
			//o << rvid << ":" << normal ;
			return o;
		}

		virtual ostream& operator<<(ostream& o) const 
		{
			//o << ":"; 
			return o;
		}

	};

	struct QuotationMark : Node
	{
		QuotationMark( const string& _jtxt, const JsonToken _jc ) : Node( _jtxt, _jc ) {}
		private:
		const string vtext () const 
		{
			const Markers& m( jc );
			const string s( Slice( jtxt, m ) );
			return s;
		}
		virtual operator string () const 
		{
			stringstream ss;
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				const TokenType t( n );
				if ( t == Character ) ss << n;
			}
			return ss.str();
		}

		virtual CBug& operator<<(CBug& o) const 
		{
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				const JsonToken& subjc( n );
				const char cc( subjc );
				o << fence << cc;
			}
			return o;
		}

		virtual ostream& operator<<(ostream& o) const 
		{
			const Markers& m( jc );
			const string& s( Slice( jtxt, m ) );
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				const JsonToken& subjc( n );
				const TokenType& tt( subjc );
				const char cc( subjc );
				if ( tt == ValueQuots )
					o << n.vtext(); 
				o << n;
			}
			return o;
		}

	};

	struct SpecialChar : Node
	{
		SpecialChar( const string& _jtxt, const JsonToken _jc ) : Node( _jtxt, _jc ) {}
		private:
		virtual CBug& operator<<(CBug& o) const 
		{
			o << red << jc << normal;
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}

		virtual ostream& operator<<(ostream& o) const 
		{
			//o << jc; 
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}

	};

	struct RegularCharacter : Node
	{
		RegularCharacter( const string& _jtxt, const JsonToken _jc ) : Node( _jtxt, _jc ) {}
		private:
		operator const bool () ;
		virtual CBug& operator<<(CBug& o) const 
		{
			o << yellow << bold << jc << normal;
			return o;
		}
		virtual ostream& operator<<(ostream& o) const 
		{
			const char cc( jc );
			o << cc;
			return o;
		}
	};

	struct ValueText : Node
	{
		ValueText( const string& _jtxt, const JsonToken _jc, const string _valuetext ) : Node( _jtxt, _jc ), valuetext( _valuetext ) {}
		private:
		virtual CBug& operator<<(CBug& o) const 
		{
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}

		virtual ostream& operator<<(ostream& o) const 
		{
			o << vtext();
			return o;
		}
		const string vtext () const { return valuetext; }
		private:
		const string valuetext;

	};

	struct Excavator 
	{
		Excavator( const string& _txt, NodeBase& _node, QueString& _qtext )
			: txt(_txt), node( _node ), qtext( _qtext ) {}
		void operator()( char c ) { qtext( c ); }
		operator Markers ()
		{
			while ( ! qtext.empty() )
			{
				const JsonToken& jc( qtext.front() );
				qtext.pop();
				const TokenType& tokentype( jc );
				const Markers& jcp( jc );

				if ( ! node( txt, qtext, jc ) )
				{
					const Markers m( node );
					Markers none( node );
					return m;
				}
			}
			Markers none( node );
			return none;
		}
		private:
		const string& txt;
		NodeBase& node;
		QueString& qtext;
		string valuestring;
		stack< JsonToken > Stack;
	};



	inline bool NodeBase::operator()( const string& txt, QueString& qtext, const JsonToken& jc )
	{
		const TokenType tokentype( jc );

		switch ( tokentype )
		{
			case ObjectOpen:
			{
				push_back( new Object( txt, jc ) );
				NodeBase& item( *back() );
				Excavator excavate( txt, item, qtext );
				Markers m( excavate );
				closure( m );
				return true;
			}
			break;
			case ObjectClose: 
			{
				push_back( new Object( txt, jc ) );
				Markers m( jc );
				closure( m );
				return false;
			}
			break;
			case ListOpen:
			{
				push_back( new List( txt, jc ) );
				NodeBase& item( *back() );
				Excavator excavate( txt, item, qtext );
				Markers m( excavate );
				closure( m );
				return true;
			}
			break;
			case ListClose: 
			{
				push_back( new List( txt, jc ) );
				Markers m( jc );
				closure( m );
				return false;
			}
			break;
			case Coln: 
			{
				if ( qtext.enquoted( false ) ) 
				{
					const Markers& m( jc );
					jc.morph( Special, ':' );
					push_back( new SpecialChar( txt, jc ) );
					return true;
				} else {
					push_back( new Object( txt, jc ) );
					return true;
				}
			}
			break;
			case Coma: 
			{
				push_back( new Comma( txt, jc ) );
				return true;
			}
			break;
			case Character: 
			{
				const char cc( jc );
				push_back( new RegularCharacter( txt, jc ) );
				return true;
			}
			break;
			case NameQuots:
			{
				if (  ! qtext.enquoted() )
				{
					push_back( new QuotationMark( txt, jc ) );
					NodeBase& item( *back() );
					Excavator excavate( txt, item, qtext );
					Markers m( excavate );
					closure( m );
					return true;
				} else {
					push_back( new QuotationMark( txt, jc ) );
					const Markers& m( jc );
					jc.swap();
					closure( m );
					return false;
				}
			}
			case ValueQuots:
			{
				if (  ! qtext.enquoted() )
				{
					push_back( new QuotationMark( txt, jc ) );
					NodeBase& item( *back() );
					Excavator excavate( txt, item, qtext );
					Markers m( excavate );
					closure( m );
					return true;
				} else {
					push_back( new QuotationMark( txt, jc ) );
					const Markers& m( jc );
					jc.swap();
					closure( m );
					return false;
				}
			}
			break;

			case Special: 
			{
				push_back( new SpecialChar( txt, jc ) );
				return true;
			}
			break;

			case ValueChar: 
			{
				stringstream ss;
				JsonToken jc2( jc );
				const char ccc( jc );
				while ( ! qtext.empty() )
				{
					const char& cc( jc2 );
					ss << cc;
					jc2=( qtext.front() );
					const TokenType& tokentype( jc2 );
					if ( tokentype != ValueChar) 
					{
						const Markers& pos( jc2 );
						jc2.swap();
						closure( pos );
						break;
					}
					qtext.pop();
				}

				
				jc.morph( ValueChar, ccc );
				const Markers& m( jc );
				jc.swap();
				jc.closure( m );
				push_back( new ValueText( txt, jc, ss.str() ) );
				return true;
			}
			break;

			case Nothing: 
				throw string( "Encountered Nothing" );
			case Root: 
			break;
		}
		return true;
	}


	struct Json
	{
		Json( const string& _jtxt ) : root( _jtxt ), jtxt( _jtxt ) {}
		operator const Object& () const
		{
			const Object* o( root );
			if ( ! o ) throw string( "Json is not loaded" );
			return *o;
		}
		operator bool ()
		{
			GlyphDisposition glyphs;
			QueString qtext( 0, glyphs );
			for ( string::const_iterator it=jtxt.begin();it!=jtxt.end();it++) 
				qtext( *it );
			Excavator excavator( jtxt, root, qtext );
			Markers m( excavator ); 
			if ( ! root ) throw string( "Cannot index json" );
			return true;
		}

		private:
		RootNode root;
		const string& jtxt;
	};

	RegularCharacter::operator const bool () 
	{
		return true;
	}


	ostream& Index::operator<<(ostream& o) const 
	{
		for ( const_iterator it=begin();it!=end();it++)
			o << it->first << "," << " ";
		return o;
	}
#if 0
	CBug& Object::operator()( CBug& o, const int level ) const
	{
		for ( Index::const_iterator ndx=index.begin();ndx!=index.end();ndx++)
		{
			const TokenType& tokentype( jc );	
			const string gt( GlyphType( jc ) );
			if ( tokentype == ObjectOpen )
			{
				continue;
			}
			if ( tokentype == ListOpen )
			{
				continue;
			}
			const string v( vtext() );
			//o.write( v.c_str(), v.size() );
			//o << gt << endl;
		}
		return o;
	}

	CBug& Object::operator<<(CBug& o) const 
	{ 	
		const Object& me( *this );
		o << me;
		return o;
	}
#endif

} // ExeJson

#endif //BUILDER_JSON_H

