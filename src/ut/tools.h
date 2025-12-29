

#ifndef TOOLS_H
#define TOOLS_H
namespace Tools
{
	inline string tabs( int j, int tw ) { stringstream ss; for ( int i=0;i<(j*tw);i++) ss<<" "; return ss.str().c_str(); }
        struct stringvector : vector<string>
        {

                virtual void split( string what ) { return split( what, "|" ); }
                virtual void split( string what, string _how )
                {
                        while ( true )
                        {
                                const size_t where( what.find( _how ) );
                                if ( where  == string::npos )
                                {
                                        if ( ! what.empty() ) push_back( what );
                                        return;
                                }
                                push_back( what.substr(0, where ) );
                                what.erase( 0, where+ 1 );
                        }
                }


                friend ostream& operator<<(ostream&,const stringvector&);
                ostream& operator<<(ostream& o) const
                {
                        for (const_iterator it=begin();it!=end();it++) o << it->c_str() << endl;
                        return o;
                }
        };

        inline ostream& operator<<(ostream& o,const stringvector& m)
                { return m.operator<<(o); }


	inline string trims( string s )
	{ 
		const size_t f( s.find_first_not_of( "\t ") );
		if ( f != string::npos ) s.erase( 0, f );
		size_t l( s.find_last_not_of( "\t ") );
		if ( l != string::npos ) {l+=1; s.erase( l, s.size() );}
		return s;
	}



struct Matcher : string
{
        Matcher() : compiled(false)  {}
        Matcher(string s) : string(s),compiled(false) { }
        ~Matcher()
        {
                if (compiled) regfree(&re);
                compiled=false;
        }
        bool operator()(string& s)
        {
                if (!compiled) compile();
                status = regexec(&re, s.c_str(), (size_t) 0, NULL, 0);
                return !status;
        }

        private:
        bool compile()
        {
                if (compiled) return true;
                status=regcomp(&re, c_str(), REG_EXTENDED|REG_NOSUB);
                //status=regcomp(&re, c_str(), REG_EXTENDED);
                //status=regcomp(&re, c_str(), 0);
                if (status)
                {
                        regfree(&re);
			return false;
                }
                compiled=true;
		return true;
        }
        bool compiled;
        int    status;
        regex_t    re;
};



} // Tools
#endif //TOOLS_H

