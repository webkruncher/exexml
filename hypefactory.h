
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
			: msprgx( "MSPDir" ),
				Yaml::yaml( _tabwidth , _tablevel, _parent) {}
		virtual yaml* generate( int _tabwidth, int _tablevel,  yamlstring line, yaml* _parent=NULL )
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
		Tools::Matcher msprgx;
	};
} // HypeFactory
#endif // HYPE_FACTORY_H

