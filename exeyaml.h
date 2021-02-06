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
	ExeYaml Jack M. Thompson WebKruncher.com
	Original creation: 11/2018
	exexml is designed to read yaml into a tight knit C++ hierarchy, and write it back out.
	Every node is based on a generic node, and can be customized.
	The only dependency is C++ / STL.
	The utility does not transform or validate.  
	exeyaml is designed to be fully contained in a single header file that should never exceed 200 lines of code.
*/

#ifndef BUILDER_YAML_H
#define BUILDER_YAML_H

namespace Yaml
{
	struct yamlstring : string
	{
		yamlstring(){} 
        yamlstring( const yamlstring& yy) : string( yy ) {}
        yamlstring& operator=( const yamlstring& yy) 
        {
            if ( this != &yy ) assign( yy );
            return *this;
        }
		int ntabs( const int tabwidth) const
		{
			int ltl( 0 );
			while ( 
				( ltl < size() ) && 
				(
					( ( (*this)[ ltl ] == ' '  ) ) || 	
					( (*this)[ ltl ] == '-' ) 
				)
			)
				ltl++;
			ltl/=tabwidth; 
			return ltl;
		}
	};

	struct stream 
	{ 
		enum UsedFlag {used=1, unused, subobj};
		stream( istream& _in ) : in( _in ), inuse( used ) { }
        stream( const stream& ss) : in( ss.in ), line( ss.line ), inuse( ss.inuse ) { }
        stream& operator=( const stream& ss) 
        {
            throw string("Don't use assignment operator for stream");
            return *this;
        }
		operator yamlstring& ()
		{
			if ( unused == inuse ) return line;
			line.clear();
			if ( in.eof() ) return line;
			getline( in, line );
			inuse=unused;
			return line;
		}
		operator bool (){ return ! in.eof() ; }
		void operator = (UsedFlag b) { inuse=b; }
		private:
		istream& in;
		yamlstring line;
		UsedFlag inuse;
	};

	struct yaml;
    struct Matter : vector<yaml*> 
    {
		Matter() {}
    private:
        Matter( const Matter& yy) : vector<yaml*>( yy ) { throw string("Don't copy matter"); }
        Matter& operator=( const Matter& ss) { throw string("Dont use assignment operator for Matter"); return *this; }
    };

	struct yaml : yamlstring
	{
		yaml( int _tabwidth=2, int _tablevel=-1, yaml* _parent=NULL )
			: tabwidth( _tabwidth ), tablevel( _tablevel ), 
				parent( _parent ) {}
        yaml( const yaml& yy) : yamlstring( yy ) { }
        yaml& operator=( const yaml& ss) 
        {
            throw string("Dont use assignment operator for yaml");
            return *this;
        }

		virtual ~yaml()
		{
			for ( vector<yaml*>::iterator it=matter.begin();it!=matter.end();it++) delete *it;
		}
		virtual yaml* generate( int _tabwidth, int _tablevel, yamlstring line, yaml* _parent=NULL) = 0;

		virtual stream& operator << ( stream& streaminput );
        operator const Matter& () const {return matter;}

		private:
		int tabwidth,tablevel;
		protected:
        Matter matter;


        friend ostream& operator<<(ostream&,const yaml&);
        virtual ostream& operator<<(ostream& o) const = 0; 
		protected:
		void path( yamlstring& p )
		{ 
			if ( parent ) parent->path( p ); 
			if ( !empty() ) { p+="/"; p+=c_str(); }
		}
		map<int, vector<yamlstring> > antimatter;
		public:
		yaml* parent;
	};
    inline ostream& operator<<(ostream& o,const yaml& m)
            { return m.operator<<(o); }



	inline stream& yaml::operator << ( stream& streaminput )
	{
		while ( streaminput )
		{
			const yamlstring& line( streaminput ); 

			const size_t hash( line.find( "#" ));
			const size_t notatab( line.find_first_not_of( " " )); 
			const string trimmed( trims( line ) ); 
			const bool comment( ( hash != string::npos ) && ( notatab == hash ) );
			const int linetablevel ( line.ntabs( tabwidth));

			if ( ( trimmed.empty() ) || ( trimmed.find("---") == 0 ) || ( comment ) )
			{
				if ( ! streaminput ) if ( trimmed.empty() ) return streaminput;
				antimatter[ antimatter.size() ].push_back( line );
				streaminput=stream::used; 
				continue;
			} 

			if ( linetablevel < tablevel ) return streaminput;

			if ( linetablevel >= tablevel )
			{
				if ( ( linetablevel == tablevel ) && ( empty() ) )
				{
					yamlstring& me( * this );
					me=line;
					streaminput=stream::used; 
				} else { 
					if ( linetablevel == tablevel ) return streaminput;
					yaml* y( generate( tabwidth, tablevel+1, line ) );
					(*y) << streaminput; 
					matter.push_back( y );
				}
			}
		}
		return streaminput;
	}



} // Yaml

#endif //BUILDER_YAML_H

#if 0
    // Example ostream operation
	inline ostream& yaml::operator<<(ostream& o) const 
	{
		map<int,vector<yamlstring> >::const_iterator found( antimatter.begin() );
		//if ( ! empty() ) o << c_str() << endl;

		int j( 0 );
		for (vector<yaml*>::const_iterator it=matter.begin();it!=matter.end();j++,it++) 
		{
			const yaml& y( **it );
			map<int,vector<yamlstring> >::const_iterator search( antimatter.find(j) );
			if ( search != antimatter.end() ) 
			{
				found=search;
				const vector<yamlstring>& y( found->second );
				for ( vector<yamlstring>::const_iterator that=y.begin();that!=y.end();that++) 
					o << (*that) << endl;
				found++;
			}
			o << y ;
		} 

		while ( found != antimatter.end() )
		{
			const vector<yamlstring>& y( found->second );
			for ( vector<yamlstring>::const_iterator that=y.begin();that!=y.end();that++) 
				o << (*that) << endl;
			found++;
		}
		return o;
	}
#endif
