
#ifndef HYPE_FACTORY_H
#define HYPE_FACTORY_H
namespace HypeFactory
{
	struct MspFactory : Yaml::yaml
	{
		MspFactory( int _tabwidth=2, int _tablevel=-1, yaml* _parent=NULL)
			: Yaml::yaml( _tabwidth , _tablevel, _parent) {}
		virtual yaml* generate( int _tabwidth, int _tablevel,  yamlstring line, yaml* _parent=NULL )
		{
			Yaml::yamlstring p; path( p ); p+=string("|")+line;
			yaml* y( new MspFactory( _tabwidth, _tablevel, ((_parent)?_parent:this ) ));
			return y;
		} 
		ostream& operator<<(ostream& o) const;
	};
	inline ostream& MspFactory::operator<<(ostream& o) const
	{
		cerr << "\033[32m" << c_str() << "\033[0m" << endl;
		return Yaml::yaml::operator<<(o);
	}

	struct ChainFactory : Yaml::yaml
	{
		ChainFactory( int _tabwidth=2, int _tablevel=-1, yaml* _parent=NULL)
			: Yaml::yaml( _tabwidth , _tablevel, _parent), msprgx( "MSPDir" ) {}
		virtual yaml* generate( int _tabwidth, int _tablevel,  yamlstring line, yaml* _parent=NULL );
		virtual ostream& operator<<(ostream& o) const ;
		Tools::Matcher msprgx;
	};

	inline ostream& ChainFactory::operator<<(ostream& o) const 
	{
		map<int,vector<yamlstring> >::const_iterator found( antimatter.begin() );
		//if ( ! empty() ) o << c_str() << endl;

		int j( 0 );
		for (vector<Yaml::yaml*>::const_iterator it=matter.begin();it!=matter.end();j++,it++) 
		{
			const Yaml::yaml& y( **it );
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

	inline Yaml::yaml* ChainFactory::generate( int _tabwidth, int _tablevel,  yamlstring line, yaml* _parent )
	{
		yaml* Y( NULL );
		Yaml::yamlstring p; path( p ); p+=string("|")+line;
		if ( msprgx( line ) )
		{
			cerr << "\033[45m\033[36m" << p  << "\033[0m" << endl;
			if ( ! Y ) Y = new MspFactory( _tabwidth, _tablevel, ((_parent)?_parent:this) );
		}
		if ( ! Y ) Y = new ChainFactory( _tabwidth, _tablevel, ((_parent)?_parent:this) );

		return Y;
	} 


} // HypeFactory
#endif // HYPE_FACTORY_H

