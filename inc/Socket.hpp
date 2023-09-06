#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "Server.hpp"
# include "dirent.h"

class Config;
class Client;
class Socket
{
    public:
        Socket( std::string config, std::string path );
        ~Socket();

		// Config			*serverConfig;
        std::string		ipAddr;
        int				fd;
		int				port;
        sockaddr_in		socketAddr;
        unsigned int	socketAddrLen;
        std::string		logFile;
		bool			bound;
		std::string		defaultHost;
        int             setupSockets();
        std::string     getLocationPage( std::string, std::string, Client * );
		std::string		getRedirectPage( std::string, std::string, Client * );
		Config			*getConfig( std::string, std::string, Client * );
		void			setRouteConfigs( std::string, std::string );
		void			setPortLogHost( std::string );

		std::map< std::string, std::vector< Config* > >	hostConfigs;/* host - configs */
		std::vector< std::string >						hosts;
		// std::map< std::string, Config* >				routes; 	/* location - config */

    private:
                    Socket() {};
                	Socket( const Socket & );
        Socket &	operator=( const Socket & );

};

#endif
