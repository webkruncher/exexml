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
	CBug() : ofstream( "/dev/stdout" ) {}
} ;

namespace ExeJson
{
	enum TokenType { 
		Root, 
		ObjectOpen, ObjectClose, ListOpen, ListClose,
		Coma, Coln, Quots, Special, Character
	};

	inline void JsonGlyphTypeLegend( ostream& o)
	{
		o<< "Root       :" << Root       << endl;
		o<< "ObjectOpen :" << ObjectOpen << endl;
		o<< "ObjectClose:" << ObjectClose<< endl;
		o<< "ListOpen   :" << ListOpen   << endl;
		o<< "ListClose  :" << ListClose  << endl;
		o<< "Coma       :" << Coma       << endl;
		o<< "Coln       :" << Coln       << endl;
		o<< "Quots      :" << Quots      << endl;
		o<< "Special    :" << Special    << endl;
		o<< "Character  :" << Character  << endl;
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
		JsonToken( const size_t _pos, TokenType _t, char _c ) : pos( _pos, 0 ), tokentype( _t ), c( _c ) {}
		JsonToken( const size_t zero, const size_t _pos, TokenType _t, char _c ) : pos( zero, _pos ), tokentype( _t ), c( _c ) {}
		JsonToken( const JsonToken& that )
			: pos( that.pos ), tokentype( that.tokentype ), c( that.c ) { }
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
		operator const char () { return c; }
		void morph( const TokenType _t, const char _c ) const
		{
			c=_c;
			tokentype=_t;
		}
		operator const TokenType () const { return tokentype; }
		operator Markers () const { return pos; }
		void closure( const Markers& _pos ) const { pos.second=_pos.second-1; }
		operator string () const
		{
			stringstream ss;
			switch ( tokentype )
			{
				case Coma: ss << green << c << normal; break;
				case Coln: ss << bluebk << white << bold << c << normal; break;
				case Quots: ss << rvid << yellow << c << normal; break;
				case ListOpen: ss << black << whitebk << c << normal; break;
				case ListClose: ss << black << whitebk << c << normal; break;
				case ObjectOpen: ss << rvid << bold << c << normal; break;
				case ObjectClose: ss << rvid << ulin << c << normal; break;
				case Special: ss << red << c << normal; break;
				case Character: ss << teal << c << normal; break;
				case Root: ss << bluebk << green << bold << c << normal; break;
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

	struct QueString : queue< JsonToken >
	{
		QueString() = delete;
		QueString( const int _much, GlyphDisposition& _glyphs ) 
			: much( _much ), glyphs( _glyphs ) {}
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
					case '"': { JsonToken jc( much, Quots, c ); push( jc ); break; }
					case ',': { JsonToken jc( much, Coma, c ); push( jc ); break; }
					case ':': { JsonToken jc( much, Coln, c ); push( jc ); break; }
					case '{': { JsonToken jc( much, ObjectOpen, c ); push( jc ); break; }
					case '}': { JsonToken jc( 0, much, ObjectClose, c ); push( jc ); break; }
					case '[': { JsonToken jc( much, ListOpen, c ); push( jc ); break; }
					case ']': { JsonToken jc( 0, much, ListClose, c ); push( jc ); break; }
					default: { JsonToken jc( much, Character, c ); push( jc ); }
				}
			}	
		}
		bool enquoted( const bool toggle=true ) { return glyphs.enquoted( toggle ); }
		private:
		int much;
		GlyphDisposition& glyphs;
	};

	struct Items : set< size_t > {};
	struct Index : map< string, Items > {};

	struct Excavator;
	struct Object;
	struct NodeBase : vector< NodeBase* >
	{
		friend struct Excavator;
		NodeBase( const int _level ) : level( _level ) {}
		NodeBase( const int _level, const JsonToken _jc ) : level( _level ), jc( _jc ) {}
		void operator = ( const size_t _endmarker ) { jc.pos.second=_endmarker; }
		virtual ~NodeBase() { for ( iterator it=begin();it!=end();it++) delete *it; }
		virtual bool operator()( const string&, QueString&, const JsonToken& );
		void closure( Markers& pos ) const { jc.closure( pos ); }
		operator Markers () const { return jc; }
		operator const TokenType () const { return jc; }
		virtual operator const bool () = 0;
		virtual operator string () const = 0;
		virtual const NodeBase& operator[]( const size_t ndx ) const = 0;
		virtual const Items& operator[]( const string& name ) const = 0;
		virtual operator const Object* () const { return nullptr; }
		protected:
		const int level;
		const JsonToken jc;
		friend ostream& operator<<(ostream&, const NodeBase&);
		virtual ostream& operator<<(ostream& o) const = 0;
		friend CBug& operator<<(CBug&, const NodeBase&);
		virtual CBug& operator<<(CBug& o) const = 0;
	};
	inline ostream& operator<<(ostream& o, const NodeBase& n ) { return n.operator<<(o); }
	inline CBug& operator<<(CBug& o, const NodeBase& n ) { return n.operator<<(o); }

	struct Node : NodeBase
	{
		Node( const int _level ) : NodeBase( _level ) {}
		Node( const int _level, const JsonToken _jc ) : NodeBase( _level, _jc ) {}
		virtual operator const bool () 
		{
			for ( iterator it=begin();it!=end();it++)
			{
				NodeBase& n( **it );
				if ( ! n ) return false;
			}
			return true;
		}
		virtual operator string () const { return ""; }

		virtual const NodeBase& operator[]( const size_t ndx ) const
		{
			const vector< NodeBase* >& me( *this );
			return *me[ ndx ];
		}
		virtual const Items& operator[]( const string& name ) const
		{
			Index::const_iterator found( index.find( name ) );
			if ( found == index.end() ) throw string("Cannot find " ) + name;
			return found->second;
		}

		protected:
		Index index;
		private:
		virtual ostream& operator<<(ostream& o) const = 0;
		virtual CBug& operator<<(CBug& o) const = 0;
	};

	
	struct RootNode : Node
	{
		RootNode() : Node( 0 ) {}
		operator const Object* () const 
		{
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				const Object* oo( n );
				if ( oo ) return oo;
			}
			return nullptr;	
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
		virtual ostream& operator<<(ostream& o) const 
		{
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}
	};



	struct Object : Node
	{
		Object( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
		operator const Index& () { return index; }
		operator const Object* () const { return this; }
		private:
		virtual operator const bool () 
		{
			bool tillcoma( false );
		
			int ndx( 0 );	
			for ( iterator it=begin();it!=end();it++,ndx++)
			{
				NodeBase& n( **it );
				if ( ! n ) return false;
				const TokenType t( n );
				if ( t == Coln ) tillcoma=true;
				if ( t == Coma ) tillcoma=false;
				if ( ! tillcoma ) 
				{
					const string name( n );
					if ( ! name.empty() ) 
					{
						cout << green << "<" << name << ">" << normal << endl;
						index[ name ].insert( ndx );
					}
				}
			}
			return true;
		}
		virtual CBug& operator<<(CBug& o) const 
		{
			o << tracetabs( level-1 ) << blue << jc << normal;
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}
		virtual ostream& operator<<(ostream& o) const 
		{
			o << jc;
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
		List( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
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
			o << jc ;
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}
	};

	struct Comma : Node
	{
		Comma( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
		private:
		virtual CBug& operator<<(CBug& o) const 
		{
			o << blink << "," << normal;
			return o;
		}
		virtual ostream& operator<<(ostream& o) const 
		{
			o << "," ;
			return o;
		}
	};

	struct Colon : Node
	{
		Colon( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
		private:
		virtual CBug& operator<<(CBug& o) const 
		{
			o << rvid << ":" << normal ;
			return o;
		}
		virtual ostream& operator<<(ostream& o) const 
		{
			o << ":"; 
			return o;
		}
	};

	struct QuotationMark : Node
	{
		QuotationMark( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
		private:
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
			o << rvid << mgenta << blink << "\"" << normal;
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}
		virtual ostream& operator<<(ostream& o) const 
		{
			o << "\"" ;
			for ( const_iterator it=begin();it!=end();it++)
			{
				const NodeBase& n( **it );
				o << n;
			}
			return o;
		}
	};

	struct SpecialChar : Node
	{
		SpecialChar( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
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
			o << jc; 
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
		RegularCharacter( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
		private:
		virtual CBug& operator<<(CBug& o) const 
		{
			o << yellow << bold << jc << normal;
			return o;
		}
		virtual ostream& operator<<(ostream& o) const 
		{
			o << jc;
			return o;
		}
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
				if ( ! node( txt, qtext, jc ) )
				{
					const Markers m( node );
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
	};

	inline bool NodeBase::operator()( const string& txt, QueString& qtext, const JsonToken& jc )
	{
		const TokenType tokentype( jc );
		switch ( tokentype )
		{
			case ObjectOpen:
			{
				push_back( new Object( level+1, jc ) );
				NodeBase& item( *back() );
				Excavator excavate( txt, item, qtext );
				Markers m( excavate );
				closure( m );
				return true;
			}
			break;
			case ObjectClose: 
			{
				push_back( new Object( level, jc ) );
				Markers m( jc );
				closure( m );
				return false;
			}
			break;
			case ListOpen:
			{
				push_back( new List( level+1, jc ) );
				NodeBase& item( *back() );
				Excavator excavate( txt, item, qtext );
				Markers m( excavate );
				closure( m );
				return true;
			}
			break;
			case ListClose: 
			{
				push_back( new List( level, jc ) );
				Markers m( jc );
				closure( m );
				return false;
			}
			break;
			case Coln: 
			{
				if ( qtext.enquoted( false ) ) 
				{
					jc.morph( Special, ':' );
					push_back( new SpecialChar( level, jc ) );
				} else {
					push_back( new Colon( level, jc ) );
				}
				return true;
			}
			break;
			case Coma: 
			{
				push_back( new Comma( level, jc ) );
				return true;
			}
			break;
			case Character: 
			{
				push_back( new RegularCharacter( level, jc ) );
				return true;
			}
			break;
			case Quots:
			{
				if (  ! qtext.enquoted() )
				{
					push_back( new QuotationMark( level+1, jc ) );
					NodeBase& item( *back() );
					Excavator excavate( txt, item, qtext );
					Markers m( excavate );
					closure( m );
					return true;
				} else {
					push_back( new QuotationMark( level+1, jc ) );
					return false;
				}
			}
			break;

			case Special: 
			{
				push_back( new SpecialChar( level, jc ) );
				return true;
			}
			case Root: 
			break;
		}
		return true;
	}


	struct Json
	{
		bool operator+=( const string& txt )
		{
			GlyphDisposition glyphs;
			QueString qtext( 0, glyphs );
			for ( string::const_iterator it=txt.begin();it!=txt.end();it++) 
				qtext( *it );
			Excavator excavator( txt, root, qtext );
			Markers m( excavator ); 
			if ( false ) { CBug cbug; cbug << root; cerr << endl << setw( 80 ) << setfill( '-' ) << "-" << endl; }
			if ( ! root ) throw string( "Cannot index json" );
			cerr << root;
			JsonGlyphTypeLegend( cout );
			return true;
		}
		operator const Object& () const
		{
			const Object* o( root );
			if ( ! o ) throw string( "Json is not loaded" );
			return *o;
		}
		private:
		RootNode root;
	};

} // ExeJson

#endif //BUILDER_JSON_H

