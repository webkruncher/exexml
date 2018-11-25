
#ifndef BUILDER_YAML_H
#define BUILDER_YAML_H

namespace Yaml
{
	struct yamlstring : string
	{
		yamlstring(){} 
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

	struct yaml : yamlstring
	{
		yaml( int _tabwidth=2, int _tablevel=-1, yaml* _parent=NULL )
			: tabwidth( _tabwidth ), tablevel( _tablevel ), 
				parent( _parent ) {}

		virtual ~yaml()
		{
			for ( vector<yaml*>::iterator it=matter.begin();it!=matter.end();it++) delete *it;
		}
		virtual yaml* generate( int _tabwidth, int _tablevel, yamlstring line, yaml* _parent=NULL) = 0;

		virtual stream& operator << ( stream& streaminput );

		private:
		int tabwidth,tablevel;
		protected:
		vector<yaml*> matter;


                friend ostream& operator<<(ostream&,const yaml&);
                virtual ostream& operator<<(ostream& o) const;
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

	inline ostream& yaml::operator<<(ostream& o) const
	{
		map<int,vector<yamlstring> >::const_iterator found( antimatter.begin() );
		if ( ! empty() ) o << c_str() << endl;

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

	inline stream& yaml::operator << ( stream& streaminput )
	{
		while ( streaminput )
		{
			const yamlstring& line( streaminput ); 

			const size_t hash( line.find( "#" ));
			const size_t notatab( line.find_first_not_of( " " )); 
			const string trimmed( Tools::trims( line ) ); 
			const bool comment( ( hash != string::npos ) && ( notatab == hash ) );
			const int linetablevel ( line.ntabs( tabwidth));

			if ( ( trimmed.empty() ) || ( trimmed.find("---") == 0 ) || ( comment ) )
			{
				if ( ! streaminput ) if ( trimmed.empty() ) return streaminput;
				antimatter[ matter.size() ].push_back( line );
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

