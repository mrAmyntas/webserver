#include "../inc/Client.hpp"

//CONSTRUCTOR + DESTRUCTOR
Client::Client( int fd, int sock_num ) : _conn_fd( fd ), _sock_num( sock_num )
{ 
    this->_timestamp = std::time(nullptr);
	this->_requestHeader = "";
	this->_requestBody = "";
	this->_requestMethod = "";
	this->_requestLocation = "";
	this->_requestHost = "";
	this->_headerSet = false;
    this->_request_is_read = true;
	this->_client_body_too_large = false;
	this->_badRequest = false;
	this->_sendAgain = false;
	this->_bytesSent = 0;
	this->_request = "";
}
Client::~Client() { std::cout << "Client removed\n"; }

//COPY CONSTRUCTOR
Client::Client( const Client& src ) { *this = src; }

//ASSIGNMENT OPERATOR
Client & Client::operator=( const Client& src )
{
    this->_conn_fd = src._conn_fd;
    this->_sock_num = src._sock_num;
    this->_timestamp = src._timestamp;
	this->_requestHeader = src._requestHeader;
	this->_requestBody = src._requestBody;
	this->_requestMethod = src._requestMethod;
	this->_requestLocation = src._requestLocation;
	this->_requestHost = src._requestHost;
	this->_headerSet = src._headerSet;
	this->_request_is_read = src._request_is_read;
	this->_client_body_too_large = src._client_body_too_large;
	this->_badRequest = src._badRequest;
	this->_sendAgain = src._sendAgain;
	this->_bytesSent = src._bytesSent;
	this->_request = src._request;
    return *this;
}

//CLIENT FUNCTIONS

//UTILS
void    Client::update_client_timestamp()
{
    this->_timestamp = std::time( nullptr );
}

//GETTERS
std::string	Client::getLocation()		{ return this->_requestLocation; }
std::string	Client::getBody()			{ return this->_requestBody; }
std::string	Client::getHeader()			{ return this->_requestHeader; }
std::string	Client::getMethod()			{ return this->_requestMethod; }
std::string	Client::getHost()			{ return this->_requestHost; }
std::string	Client::getRequest()		{ return this->_request; }
int			Client::getConnectionFD()	{ return this->_conn_fd; }
int			Client::getSockNum()		{ return this->_sock_num; }
int			Client::getBytesSent()		{ return this->_bytesSent; }
bool		Client::requestIsRead()		{ return this->_request_is_read; }
bool		Client::headerIsSet()		{ return this->_headerSet; }
bool		Client::bodyTooLarge()		{ return this->_client_body_too_large; }
bool		Client::badRequest()		{ return this->_badRequest; }
bool		Client::sendAgain()			{ return this->_sendAgain; }
std::time_t	Client::getTimeStamp()		{ return this->_timestamp; }

//SETTERS
void	Client::setBody( std::string body )					{ this->_requestBody = body; }
void	Client::setBytesSent( int bytesSent )				{ this->_bytesSent = bytesSent; }
void	Client::setHeader( std::string header, int end )	{ this->_requestHeader = header; this->_requestHeader[end] = '\0'; }
void	Client::setMethod( std::string method )				{ this->_requestMethod = method; }
void	Client::setLocation( std::string location )			{ this->_requestLocation = location; }
void	Client::setRequest( std::string request )			{ this->_request = request; }
void	Client::setHeaderIsSet( bool status )				{ this->_headerSet = status; }
void	Client::setRequestIsRead( bool status )				{ this->_request_is_read = status; }
void	Client::setBodyTooLarge( bool status )				{ this->_client_body_too_large = status; }
void	Client::setBadRequest( bool status )				{ this->_badRequest = status; }
void	Client::setSendAgain( bool status )					{ this->_sendAgain = status; }

void	Client::setHost( std::string header )
{
	size_t		start, end;
	std::string	line;

	start = header.find( "Host: " );
	if ( start == std::string::npos )
		_requestHost = "" ;
	else
	{
		start = start + 6;
		end = header.find( "\r\n", start );
		line = header.substr( start, end - start );
		end = line.find( ":" );
		if ( end == std::string::npos )
			_requestHost = line;
		else
			_requestHost = line.substr( 0, end );
	}
}
