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
	enum TokenType { None, Coma, Coln, Special, Character, ObjectOpen, ObjectClose, ListOpen, ListClose };
	struct Markers : pair<size_t,size_t>
	{
		typedef pair<size_t,size_t> mtype;
		Markers( const size_t _first, const size_t _second ) : mtype( _first, _second ) {}
	};

	struct QueString;
	struct JsonToken
	{
		JsonToken( ) : pos( 0 ), tokentype( None ), c( 0 ) {}
		JsonToken( const size_t _pos, TokenType _t, char _c ) : pos( _pos ), tokentype( _t ), c( _c ) {}
		operator const char () { return c; }
		void morph( const TokenType _t, const char _c ) const
		{
			c=_c;
			tokentype=_t;
		}
		operator const TokenType () const { return tokentype; }
		operator string () const
		{
			stringstream ss;
			switch ( tokentype )
			{
				case Coma: ss << green << pos << "#" << c << normal; break;
				case Coln: ss << red << pos << "#" << c << normal; break;
				case ListOpen: ss << bold << pos << "#" << c << normal; break;
				case ListClose: ss << ulin << pos << "#" << c << normal; break;
				case ObjectOpen: ss << rvid << bold << pos << "#" << c << normal; break;
				case ObjectClose: ss << rvid << ulin << pos << "#" << c << normal; break;
				case Special: ss << yellow << pos << "#" << c << normal; break;
				default: ss << pos << "#" << c;
			}
			return ss.str();
		}
		private:
		const size_t pos;
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
		NodeBase( const int _level ) : level( _level ) {}
		virtual ~NodeBase() {}
		virtual bool operator()( const string&, QueString&, const JsonToken& ); 
		protected:
		const int level;
		private:
		virtual operator bool () = 0;
	};

	struct Node : NodeBase
	{
		Node() : NodeBase( 0 ) {}
		Node( const int _level ) : NodeBase( _level ) {}
		virtual ~Node() { for ( iterator it=begin();it!=end();it++) delete *it; }
		virtual bool operator()( const string&, QueString&, const JsonToken&);
		virtual operator bool () {return true;}
	};


	inline bool NodeBase::operator()( const string& txt, QueString& qtext, const JsonToken& c )
	{
		const TokenType tokentype( c );
		string s(c);
		cbug << s;
		return true;
	}


	struct Object : Node
	{
		Object() : Node( 0 ) {}
		Object( const int _level ) : Node( _level ) {}
		virtual operator bool () {return true;}
	};

	struct List : Node
	{
		List() : Node( 0 ) {}
		List( const int _level ) : Node( _level ) {}
		virtual operator bool () {return true;}
	};


	struct Excavator 
	{
		Excavator( const string& _txt, NodeBase& _node, QueString& _qtext ) : txt(_txt), node( _node ), qtext( _qtext ) {}
		void operator()( char c ) { qtext( c ); }
		operator bool ()
		{
			while ( ! qtext.empty() )
			{
				const JsonToken c( qtext.front() );
				if ( ! node( txt, qtext, c ) ) return false;
				if ( ! qtext.empty() ) qtext.pop();
			}
			return node;
		}
		private:
		const string& txt;
		NodeBase& node;
		QueString& qtext;
	};



	inline bool Node::operator()( const string& txt, QueString& qtext, const JsonToken& c )
	{
		const TokenType tokentype( c );
		switch ( tokentype )
		{
			case ObjectOpen:
			{
				//cbug << endl << "<" << level+1 <<";";
				push_back( new Object( level+1 ) );
				qtext.pop();
				Excavator excavate( txt, *back(), qtext );
				if ( ! excavate ) return false;
			}
			case ObjectClose: 
			{
				if ( ! qtext.empty() ) 
				{
					//cbug << level << ">" << ";" << endl;
					qtext.pop();
				}
				return true;
			}
			case ListOpen:
			{
				//cbug << "|" << level+1 <<";";
				push_back( new List( level+1 ) );
				qtext.pop();
				Excavator excavate( txt, *back(), qtext );
				if ( ! excavate ) return false;
			}
			case ListClose: 
			{
				if ( ! qtext.empty() ) 
				{
					//cbug << level << "|" << ";";
					qtext.pop();
				}
				return true;
			}
			default: return NodeBase::operator()( txt, qtext, c );
		}
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
			return true;
		}
		private:
		Object root;
	};

} // ExeJson

#endif //BUILDER_JSON_H

