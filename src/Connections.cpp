#include "../inc/Server.hpp"

int	Server::uniqueSocket( std::string config, std::string & host )
{
	size_t		start, end;
	std::string	line;
	int			port;

	start = config.find( "listen" ) + 7;
	end = config.find( '\n', start );
	line = config.substr( start, end - start );
	port = std::stoi( line );
	start = line.find( " " );
	if ( start != std::string::npos )
		host = line.substr( start + 1 );
	else
		host = "localhost";
	for ( std::vector<Socket *>::iterator it = this->_sockets.begin(); it != this->_sockets.end(); it++ ) {
		if ( port == (*it)->port )
			return port;
	}
	return 0;
}

void	Server::addHost( int port, std::string host, std::string config, std::string path )
{
	std::vector< Socket* >::iterator		it;
	std::vector< std::string >::iterator	it2;
	std::vector< Config *>					vconfig, vconfig2;

	for ( it = this->_sockets.begin(); it != this->_sockets.end(); it++ ) {
		if ( (*it)->port == port )
		{
			for ( it2 = (*it)->hosts.begin(); it2 != (*it)->hosts.end(); it2++ ) {
				if ( *it2 == host )
					return ;
			}
			(*it)->hosts.push_back( host );
			Config *new_config = new Config( config, path );
			vconfig.push_back( new_config );
			(*it)->hostConfigs.insert( std::make_pair( host, vconfig ) );
			if ( host == "localhost")
			{
				Config *new_config2 = new Config( config, path );
				vconfig2.push_back( new_config2 );
				(*it)->hostConfigs.insert( std::make_pair( "127.0.0.1", vconfig2 ) );
			}
			else if ( host == "127.0.0.1" )
			{
				Config *new_config2 = new Config( config, path );
				vconfig2.push_back( new_config2 );
				(*it)->hostConfigs.insert( std::make_pair( "localhost", vconfig2 ) );
			}
			(*it)->setRouteConfigs( config, host );
		}
	}
}

int	Server::getSocket( int fd )
{
    for ( size_t i = 0; i < this->_sockets.size(); i++ )
        if ( fd == this->_sockets[i]->fd )
            return i;
    return -1;
}

Client *Server::getClient( int c_fd )
{
	std::vector<Client*>::iterator it;

    for( it = this->_clients.begin(); it != this->_clients.end(); ++it )
        if ( c_fd == (*it)->getConnectionFD() )
            break ;
	if ( it == this->_clients.end() )
		return nullptr;
	return	( *it );
}

int	Server::closeConnection( Client *client )
{
	if ( client == nullptr )
		return 0;
    std::vector<Client*>::iterator	it;
    std::vector<Client*>::iterator	end = this->_clients.end();
	int	fd = client->getConnectionFD();

    for( it = this->_clients.begin(); it != end; ++it )
    {
        if ( *it == client )
        {
            this->_clients.erase( it );
            delete client;
			break ;
        }
    }
    if ( close( fd ) == -1 )
		return errno; 
    std::cout << "disconnected client from socket:" << fd << std::endl;
    return 0;
}

void    Server::bounceTimedOutClients()
{
    std::vector<Client*>::iterator client = this->_clients.begin();
    std::vector<Client*>::iterator end = this->_clients.end();
    time_t current_time = std::time( nullptr );

    while ( client != end )
    {
        if ( ( *client )->getTimeStamp() + TIMEOUT <=  current_time )
        {
            std::cout << "Bouncing client from:" << ( *client )->getConnectionFD() << std::endl;
			if ( closeConnection( *client ) == EBADF )
				return ;
			client = this->_clients.begin();
			end = this->_clients.end();
        }
		else
			client++;
    }
}
