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
} cbug;

namespace ExeJson
{
	enum TokenType { 
		None, 
		ObjectOpen, ObjectClose, ListOpen, ListClose,
		Coma, Coln, Quots, Special, Character
	};

	inline void JsonGlyphTypeLegend()
	{
		cbug << "None       :" << None       << endl;
		cbug << "ObjectOpen :" << ObjectOpen << endl;
		cbug << "ObjectClose:" << ObjectClose<< endl;
		cbug << "ListOpen   :" << ListOpen   << endl;
		cbug << "ListClose  :" << ListClose  << endl;
		cbug << "Coma       :" << Coma       << endl;
		cbug << "Coln       :" << Coln       << endl;
		cbug << "Quots      :" << Quots      << endl;
		cbug << "Special    :" << Special    << endl;
		cbug << "Character  :" << Character  << endl;
	}


	struct GlyphDisposition
	{
		GlyphDisposition() : Enquoted( false ) {}
		bool enquoted()
		{
			const bool ret( Enquoted );
			Enquoted=!Enquoted;
			return ret;
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
			return o;
		}
	}; 

	inline ostream& operator<<(ostream& o,const Markers& m) { return m.operator<<(o); }

	struct QueString;
	struct NodeBase;
	struct JsonToken
	{
		JsonToken( ) : pos( 0, 0 ), tokentype( None ), c( 0 ) {}
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
		void closure( const Markers& _pos ) const { pos.second=_pos.second; }
		operator string () const
		{
			stringstream ss;
			switch ( tokentype )
			{
				case Coma: ss << green << pos << "#" << c << normal; break;
				case Coln: ss << bold << pos << "#" << c << normal; break;
				case Quots: ss << rvid << yellow << pos << "#" << c << normal; break;
				case ListOpen: ss << red << pos << "#" << "LO" << normal; break;
				case ListClose: ss << ulin << pos << "#" << "LC" << normal; break;
				case ObjectOpen: ss << rvid << bold << pos << "#" << "OO" << normal; break;
				case ObjectClose: ss << rvid << ulin << pos << "#" << "OC" << normal; break;
				case Special: ss << yellow << pos << "#" << c << normal; break;
				case Character: ss << teal << pos << "#" << c << normal; break;
				default: ss << pos << "#" << c;
			}
			return ss.str();
		}
		private:
		friend struct NodeBase;
		mutable Markers pos;
		mutable TokenType tokentype;
		mutable char c;
	};

	struct QueString : queue< JsonToken >
	{
		QueString() = delete;
		QueString( const int _much, GlyphDisposition& _glyphs ) 
			: much( _much ), glyphs( _glyphs ) {}
		void operator()( const char c )
		{
			much++;
			if ( empty() )
			{
				if ( c == '{' ) 
				{
					JsonToken jc( much, ObjectOpen, c );
					push(jc);
				} else {
					//JsonToken jc( much, Character, c );
					//push(jc);
				}
				return;
			}
			const char b4( back() );
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
					//default: { JsonToken jc( much, Character, c ); push( jc ); }
				}
			}	
		}
		bool enquoted() { return glyphs.enquoted(); }
		private:
		int much;
		GlyphDisposition& glyphs;
	};

	struct Excavator;
	struct NodeBase : vector< NodeBase* >
	{
		friend struct Excavator;
		NodeBase() : level( 0 ) {}
		NodeBase( const int _level, const JsonToken _jc ) : level( _level ), jc( _jc ) {}
		void operator = ( const size_t _endmarker ) { jc.pos.second=_endmarker; }
		virtual ~NodeBase() { for ( iterator it=begin();it!=end();it++) delete *it; }
		virtual bool operator()( const string&, QueString&, const JsonToken& );
		void closure( Markers& pos ) const { jc.closure( pos ); }
		protected:
		const int level;
		const JsonToken jc;
		private:
		friend ostream& operator<<(ostream&,const NodeBase&);
		virtual ostream& operator<<( ostream& ) const = 0;
	};
	inline ostream& operator<<(ostream& o,const NodeBase& n) { return n.operator<<(o); }

	struct Node : NodeBase
	{
		Node()  {}
		Node( const int _level, const JsonToken _jc ) : NodeBase( _level, _jc ) {}
		private:
		virtual ostream& operator<<( ostream& o ) const 
		{
			const string ss( jc );
			o << level << "->" << ss << " " ;
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
		Object(){}
		Object( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
	};


	struct List : Node
	{
		List( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
	};

	struct PlainCharacter : Node
	{
		PlainCharacter( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
	};

	struct Comma : Node
	{
		Comma( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
	};

	struct QuotationMark : Node
	{
		QuotationMark( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
	};

	struct SpecialChar : Node
	{
		SpecialChar( const int _level, const JsonToken _jc ) : Node( _level, _jc ) {}
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
					Markers m( jc );
					return m;
				}
			}
			Markers none( 0, 0 );
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
				return false;
			}
			break;
#if 0
			case Coma: 
			{
				push_back( new Comma( level, jc ) );
				NodeBase& item( *back() );
				Excavator excavate( txt, item, qtext );
				Markers m( excavate );
				return true;
			}
			break;
			case Quots:
			{
				if (  qtext.enquoted() )
				{
					push_back( new QuotationMark( level+1, jc ) );
					NodeBase& item( *back() );
					Excavator excavate( txt, item, qtext );
					Markers m( excavate );
					closure( m );
					return true;
				} else {
					return false;
				}
			}
			break;
			case Special: 
			{
				push_back( new SpecialChar( level, jc ) );
				NodeBase& item( *back() );
				Excavator excavate( txt, item, qtext );
				Markers m( excavate );
				return true;
			}
			break;
#endif
			
			//default: const string cc( jc ); cout << "D:" << cc << "; ";
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
			cbug << root;
			//JsonGlyphTypeLegend();
			return true;
		}
		private:
		Object root;
	};

} // ExeJson

#endif //BUILDER_JSON_H

