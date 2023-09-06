#include "../inc/Socket.hpp"

//each serverblock listens to a socket.
//The socket objects bind with the server and listen to the port
//Each object creates its own configuration file
Socket::Socket( std::string config, std::string path ) : bound( false )
{
	std::vector<Config*> configv_1, configv_2;

	Config *newConfig = new Config( config, path );
	this->setPortLogHost( config );
	configv_1.push_back( newConfig );
	this->hostConfigs.insert( std::make_pair( this->hosts[0], configv_1 ) );
	if ( this->hosts.size() > 1 )
	{
		Config *newConfig_2 = new Config( *newConfig );
		configv_2.push_back( newConfig_2 );
		this->hostConfigs.insert( std::make_pair( this->hosts[1], configv_2 ) );
	}
	ipAddr = "localhost";
	this->setupSockets();
	if ( this->bound == false )
		return ;
	this->setRouteConfigs( config, this->hosts[0] );
}

Socket::Socket( const Socket & ) { }
Socket &	Socket::operator=( const Socket & ) { return *this; }
Socket::~Socket()
{
	//delete configs
	std::map< std::string, std::vector< Config* > >::iterator	it;

	for ( it = this->hostConfigs.begin(); it != this->hostConfigs.end(); ++it ) {
		for ( size_t i = 0; i < (*it).second.size(); ++i ) {
			delete (*it).second[i];
		}
	}
    std::cout << "Socket:" << this->fd << " - bound to port:" << port << " closed\n";
    close( fd );
}

//Creates socket, fills in sockaddr_in struct in order to bind socket to address, and makes server listen to this socket
int Socket::setupSockets()
{
    this->fd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( this->fd == -1 )
        return printerror("error: socket\n" );
    int status = fcntl( this->fd, F_SETFL, O_NONBLOCK );	
    if ( status == -1 )
        printerror( "fcntl failed" );
    memset( &socketAddr, 0, sizeof( socketAddr ) );
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    socketAddr.sin_port = htons( port );
    if ( bind( this->fd, ( struct sockaddr* )&socketAddr, sizeof( socketAddr ) ) )
    {
        std::cerr << "bind failed: " << strerror( errno ) << std::endl;
        close ( this->fd );
		return 0;
    }
	int i = 1;
	setsockopt( this->fd, SOL_SOCKET, SO_REUSEADDR, ( char* )&i, sizeof( int ) );
	this->bound = true;
    if ( listen( this->fd, 100 ) )
        return printerror( "error: listen\n" );
    return 0;
}

//sets up a map with all the info for all the routes
//key = location, value = pointer to the config
void	Socket::setRouteConfigs( std::string configfile, std::string host )
{
	std::map< std::string, std::vector< Config* > >::iterator it, it2;
	std::string	location, route, host_2 = "";
	size_t		start = 0, end = 0;

	if ( host == "localhost" )
		host_2 = "127.0.0.1";
	if ( host == "127.0.0.1" )
		host_2 = "localhost";
	if ( host_2 != "" )
		it2 = this->hostConfigs.find( host_2 );
	it = this->hostConfigs.find( host );
	if ( it == this->hostConfigs.end() || ( host_2 != "" && it2 == this->hostConfigs.end() ) )
		printerror( "couldnt find host(setRouteConfigs)");
	while ( ( start = configfile.find( "location", end ) ) != std::string::npos )
	{
		Config	*routeConfig = new Config( *it->second[0] );
		end = configfile.find( "{", start ) - 1;
		location = configfile.substr( start + 9, end - ( start + 9 ) );
		end = configfile.find( "}", start );
		route = configfile.substr( start, ( end - start ) );
		routeConfig->setConfig( route );
		routeConfig->setRedirects( route, location );
		if ( host_2 != "" )
		{
			Config	*routeConfig2 = new Config( *routeConfig );
			(*it2).second.push_back( routeConfig2 );
		}
		(*it).second.push_back( routeConfig );
	}
}

//finds the correct config corresponding to the (clients) requested location
//if the client tries to upload, the request will be routed to our .pl script
//and the location needs to be the root directory , not the name itself
Config	*Socket::getConfig( std::string location, std::string host, Client *client )
{
	std::map< std::string, std::vector< Config * > >::const_iterator	it;
	std::vector< Config * >::const_iterator	it2;
	std::string	new_location = location;
	size_t		pos;

	// std::cout << "location:" << location << "   host:" << host << std::endl;

	// for ( std::vector<std::string>::const_iterator it = this->hosts.begin(); it != this->hosts.end(); it++ ) {
	// 	std::cout << "host:";
	// 	std::cout << *it << std::endl;
	// }

	it = this->hostConfigs.find( host );
	if ( it == this->hostConfigs.end() )
	{
		client->setBadRequest( true );
		return this->getConfig( location, this->defaultHost, client );
	}
	pos = location.find( it->second[0]->extension );
	if ( pos != std::string::npos )
	{
		pos = location.find_last_of( "/" ) + 1;
		new_location = location.substr( 0, pos );
	}
	for ( it2 = it->second.begin(); it2 != it->second.end(); it2++ ) {
		if ( (*it2)->location == new_location )
			return *it2;
	}
	pos = location.find( ".html" );
	if ( pos != std::string::npos )
	{
		pos = location.find_last_of( "." );
		new_location = location.substr( 0, pos );
	}

	for ( it2 = it->second.begin(); it2 != it->second.end(); it2++ ) {
		if ( (*it2)->location == new_location )
			return *it2;
	}
	return it->second[0];
}

//find the correct page for the requested location and if root is set in a route it will be replaced
std::string Socket::getLocationPage( std::string location, std::string host, Client *client )
{
    std::map<std::string, std::string>::iterator    it;
	Config	*config = this->getConfig( location, host, client );
	size_t	pos;

    it = config->pages.find( location );
    if ( it == config->pages.end() )
        return "";
	if ( config->root != "" )
	{
		pos = it->second.find( "public_html" );
		if ( pos != std::string::npos )
			it->second.replace( pos, 11, config->root );
	}
    return it->second;
}

//find the correct page to redirect to
std::string Socket::getRedirectPage( std::string location, std::string host, Client *client )
{
    std::map<std::string, std::string>::iterator	it;
	Config	*config = this->getConfig( location, host, client );

    it = config->redirects.find( location );
	if ( it == config->redirects.end() )
        return "";
    return it->second;
}

void	Socket::setPortLogHost( std::string config )
{
	size_t		start, end;
	std::string	line, host;

	start = config.find( "listen" ) + 7;
	end = config.find( '\n', start );
	line = config.substr( start, end - start );
	this->port = std::stoi( line );
	this->logFile = "logs/port" + std::to_string( this->port ) + ".log";
	start = line.find( " " );
	if ( start != std::string::npos )
	{
		this->defaultHost = line.substr( start + 1 );
		this->hosts.push_back( line.substr( start + 1 ) );
		if ( this->defaultHost == "localhost" )
			this->hosts.push_back( "127.0.0.1" );
		if ( this->defaultHost == "127.0.0.1" )
			this->hosts.push_back( "localhost" );
	}
	else
	{
		this->defaultHost = "localhost";
		this->hosts.push_back( "localhost" );
		this->hosts.push_back( "127.0.0.1" );
	}
}
