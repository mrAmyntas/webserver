#include "../inc/Server.hpp"

int main( int argc, char **argv, char **env )
{
    Server      webserv;
    std::string path;
    
    if ( argc < 1 || argc > 2 )
        return printerror( "Wrong number of arguments: " );
    
    // find the current path
    for ( int i = 0; env[i]; i++ )
    {
        path = env[i];
        if ( !path.find( "PWD=", 0 ) )
            break;
        path.erase();
    }

	try
	{
		// start the webserver with given configfile or default configfile
		if ( argc == 2 )
			webserv.startServer( argv[1], path.substr( 4, path.length() ) );
		else
			webserv.startServer( "config/default.conf", path.substr( 4, path.length() ) );
	}
	catch( const std::exception& e )
	{
		std::cerr << "error:" << e.what() << std::endl;
		return 1;
	}
    return 0;
}
