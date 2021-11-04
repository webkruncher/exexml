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
	CBug() : ofstream( "/dev/null" ) {}
} cbug;

namespace ExeJson
{
	enum TokenType { None, Coma, Coln, Special, Character, ObjectOpen, ObjectClose, ListOpen, ListClose };
	struct Markers : pair<size_t,size_t>
	{
		typedef pair<size_t,size_t> mtype;
		Markers( const size_t _first, const size_t _second ) : mtype( _first, _second ) {}
		private:
		friend ostream& operator<<(ostream&,const Markers&);
		ostream& operator<<(ostream& o) const
		{
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
		void closure( const size_t closed ) const { pos.second=closed; }
		operator const TokenType () const { return tokentype; }
		operator string () const
		{
			stringstream ss;
			switch ( tokentype )
			{
				case Coma: ss << green << pos << "#" << c << normal; break;
				case Coln: ss << bold << pos << "#" << c << normal; break;
				case ListOpen: ss << red << pos << "#" << "LO" << normal; break;
				case ListClose: ss << ulin << pos << "#" << "LC" << normal; break;
				case ObjectOpen: ss << rvid << bold << pos << "#" << "OO" << normal; break;
				case ObjectClose: ss << rvid << ulin << pos << "#" << "OC" << normal; break;
				case Special: ss << yellow << pos << "#" << c << normal; break;
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
		QueString( const int _much ) : much( _much ) {}
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
					case ',': { JsonToken jc( much, Coma, c ); push( jc ); break; }
					case ':': { JsonToken jc( much, Coln, c ); push( jc ); break; }
					case '{': { JsonToken jc( much, ObjectOpen, c ); push( jc ); break; }
					case '}': { JsonToken jc( much, ObjectClose, c ); push( jc ); break; }
					case '[': { JsonToken jc( much, ListOpen, c ); push( jc ); break; }
					case ']': { JsonToken jc( much, ListClose, c ); push( jc ); break; }
					//default: { JsonToken jc( much, Character, c ); push( jc ); }
				}
			}	
		}
		private:
		int much;
	};



	struct Excavator;
	struct NodeBase : vector< NodeBase* >
	{
		friend struct Excavator;
		NodeBase() : level( 0 ) {}
		NodeBase( const int _level, const JsonToken _jc ) : level( _level ), jc( _jc ) {}
		void operator = ( const size_t _endmarker ) { jc.pos.second=_endmarker; }
		virtual ~NodeBase() { for ( iterator it=begin();it!=end();it++) delete *it; }
		virtual bool operator()( const string&, QueString&, const JsonToken&, const size_t );
		void closure( const size_t closed ) const { jc.closure( closed ); }
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


	struct Excavator 
	{
		Excavator( const string& _txt, NodeBase& _node, QueString& _qtext ) 
			: txt(_txt), node( _node ), qtext( _qtext ) {}
		void operator()( char c ) { qtext( c ); }
		operator bool ()
		{
			while ( ! qtext.empty() )
			{
				const JsonToken& jc( qtext.front() );
				qtext.pop();
				if ( ! node( txt, qtext, jc, qtext.size() ) ) 
					return false;
			}
			return true;
		}
		private:
		const string& txt;
		NodeBase& node;
		QueString& qtext;
	};



	inline bool NodeBase::operator()( const string& txt, QueString& qtext, const JsonToken& jc, const size_t closure )
	{
		const TokenType tokentype( jc );
		switch ( tokentype )
		{
			case ObjectOpen:
			{
				push_back( new Object( level+1, jc ) );
				Excavator excavate( txt, *back(), qtext );
				if ( ! excavate ) return false;
				NodeBase& closing( *back() );
				closing.closure( closure );
			}
			break;
			case ObjectClose: 
			{
				return true;
			}
			break;
			case ListOpen:
			{
				push_back( new List( level+1, jc ) );
				Excavator excavate( txt, *back(), qtext );
				if ( ! excavate ) return false;
				NodeBase& closing( *back() );
				closing.closure( closure );
			}
			break;
			case ListClose: 
			{
				return true;
			}
			break;
			default: 
			{
				return true;
				//return NodeBase::operator()( txt, qtext, c );
			}
		}
return true;
		return false;
	}

	struct Json
	{
		bool operator+=( const string& txt )
		{
			QueString qtext( 0 );
			for ( string::const_iterator it=txt.begin();it!=txt.end();it++) 
				qtext( *it );
			Excavator excavator( txt, root, qtext );
			if ( ! excavator ) return false;
			cerr << root;
			return true;
		}
		private:
		Object root;
	};

} // ExeJson

#endif //BUILDER_JSON_H

