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

namespace ExeJson
{
	enum TokenType { None, Coma, Coln, Special, Character, ObjectOpen, ObjectClose, ListOpen, ListClose };
	struct JsonToken
	{
		JsonToken( ) : tokentype( None ), c( 0 ) {}
		JsonToken( TokenType _t, char _c ) : tokentype( _t ), c( _c ) {}
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
				case Coma: ss << green << c << normal; break;
				case Coln: ss << red << c << normal; break;
				case ListOpen: ss << bold << c << normal; break;
				case ListClose: ss << ulin << c << normal; break;
				case ObjectOpen: ss << rvid << bold << c << normal; break;
				case ObjectClose: ss << rvid << ulin << c << normal; break;
				case Special: ss << yellow << c << normal; break;
				default: ss << c;
			}
			return ss.str();
		}
		private:
		mutable TokenType tokentype;
		mutable char c;
	};

	struct QueString : queue< JsonToken >
	{
		void operator()( const char c )
		{
			if ( empty() )
			{
				if ( c == '{' ) 
				{
					JsonToken jc( ObjectOpen, c );
					push(jc);
				} else {
					JsonToken jc( Character, c );
					push(jc);
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
					case ',': { JsonToken jc( Coma, c ); push( jc ); break; }
					case ':': { JsonToken jc( Coln, c ); push( jc ); break; }
					case '{': { JsonToken jc( ObjectOpen, c ); push( jc ); break; }
					case '}': { JsonToken jc( ObjectClose, c ); push( jc ); break; }
					case '[': { JsonToken jc( ListOpen, c ); push( jc ); break; }
					case ']': { JsonToken jc( ListClose, c ); push( jc ); break; }
					default: { JsonToken jc( Character, c ); push( jc ); }
				}
			}	
		}

	};

	struct Excavator;
	struct NodeBase : vector< NodeBase* >
	{
		NodeBase() : level( 0 ) {}
		NodeBase( const int _level ) : level( _level ) {}
		virtual bool operator()( QueString&, const JsonToken& ); 
		protected:
		const int level;
	};

	struct Node : NodeBase
	{
		Node() : NodeBase( 0 ) {}
		Node( const int _level ) : NodeBase( _level ) {}
		~Node() { for ( iterator it=begin();it!=end();it++) delete *it; }
		virtual bool operator()( QueString& e, const JsonToken& c );
	};


	inline bool NodeBase::operator()( QueString& txt, const JsonToken& c )
	{
		const TokenType tokentype( c );
		string s(c);
		//cerr << s;
		return true;
	}


	struct Object : Node
	{
		Object() : Node( 0 ) {}
		Object( const int _level ) : Node( _level ) {}
	};

	struct List : Node
	{
		List() : Node( 0 ) {}
		List( const int _level ) : Node( _level ) {}
	};


	struct Excavator 
	{
		Excavator( NodeBase& _node, QueString& _text ) : node( _node ), text( _text ) {}
		void operator()( char c ) { text( c ); }
		operator bool ()
		{
			while ( ! text.empty() )
			{
				const JsonToken c( text.front() );
				node( text, c );
				if ( ! text.empty() ) text.pop();
			}
			return true;
		}
		private:
		NodeBase& node;
		QueString& text;
	};



	inline bool Node::operator()( QueString& txt, const JsonToken& c )
	{
		const TokenType tokentype( c );
		//string s(c);
		//cerr << s;
		switch ( tokentype )
		{
			case ObjectOpen:
			{
				//cerr << endl << "<" << level+1 <<";";
				push_back( new Object( level+1 ) );
				txt.pop();
				Excavator excavate( *back(), txt );
				if ( !excavate ) return false;
			}
			case ObjectClose: 
			{
				if ( ! txt.empty() ) 
				{
					//cerr << level << ">" << ";" << endl;
					txt.pop();
				}
				return true;
			}
			case ListOpen:
			{
				//cerr << "|" << level+1 <<";";
				push_back( new List( level+1 ) );
				txt.pop();
				Excavator excavate( *back(), txt );
				if ( !excavate ) return false;
			}
			case ListClose: 
			{
				if ( ! txt.empty() ) 
				{
					//cerr << level << "|" << ";";
					txt.pop();
				}
				return true;
			}
			default: NodeBase::operator()( txt, c );
		}
		return false;
	}

	struct Json
	{
		bool operator+=( const string& txt )
		{
			QueString text;
			for ( string::const_iterator it=txt.begin();it!=txt.end();it++) 
				text( *it );
			Excavator excavator( root, text );
			if ( !excavator ) return false;
			return true;
		}
		private:
		Object root;
	};

} // ExeJson

#endif //BUILDER_JSON_H

