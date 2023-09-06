#include "../inc/Server.hpp"

Client *Server::acceptRequest( int sock_num )
{
    int conn_fd = accept( this->_sockets[sock_num]->fd,
                        ( struct sockaddr* )&this->_sockets[sock_num]->socketAddr,
                        ( socklen_t * )&this->_sockets[sock_num]->socketAddrLen );
    if ( conn_fd == -1 )
	{
        printerror( "error: accept\n" );
		return nullptr ;
	}
    Client *newclient = new Client( conn_fd, sock_num );
    this->_clients.push_back( newclient );
    int status = fcntl( conn_fd, F_SETFL, O_NONBLOCK );
    if ( status == -1 )
        printerror( "fcntl failed:" );
    return this->_clients.back();
}

int Server::receiveClientRequest( Client *client )
{
    ssize_t				bytesRead;
	int					c_fd = client->getConnectionFD();
	int					sock_num = client->getSockNum();
    std::ofstream		ofs;
	std::vector<char>	buff( 1024 * 1024 );
	std::string			request;

	clearRequest( client );
	bytesRead = recv( c_fd, &buff[0], buff.size(), 0 );
	if ( bytesRead == 0 )
		return STOP_READ;
	if ( bytesRead == -1 )
	{
		closeConnection( client );
		return STOP_READ;
	}
	client->update_client_timestamp();
	request = client->getRequest();
	for ( int i = 0; i < bytesRead; ++i ) {
		request.push_back( buff[i] );
	}
	client->setRequest( request );
	//save request in logfile
    ofs.open( this->_sockets[sock_num]->logFile, std::fstream::out | std::fstream::app );
    ofs << request;
    ofs.close();
    parseRequest( client );
	if ( client->requestIsRead() == false && client->bodyTooLarge() == false )
		return CONT_READ;
	if ( client->bodyTooLarge() == true || client->badRequest() == true )
		return STOP_READ;
    return 0;
}

// remove the temporary response files
void    removeResponseFiles( void )
{
    DIR	*directory;

    directory = opendir( "response" );
    if ( !directory )
        printerror("can not open directory(removeResonseFiles): ");
    for ( struct dirent *dirEntry = readdir( directory ); dirEntry; dirEntry = readdir( directory ) )
    {
        std::string link = std::string( dirEntry->d_name );
        if ( link != "." && link != ".." )
        {
            link = "response/" + link;
            std::remove( link.c_str() );
        }
    }
	closedir( directory );
}

void	Server::resetPages( ) 
{
	std::map< std::string, std::vector< Config* > >::iterator it;
	std::string	path;

	for ( size_t i = 0; i < this->_sockets.size(); ++i ) {
		for ( it = this->_sockets[i]->hostConfigs.begin(); it != this->_sockets[i]->hostConfigs.end(); ++it ) {
			for ( size_t j = 0; j < it->second.size(); ++j ) {
				path = this->_path + "/" + it->second[j]->root;
				it->second[j]->pages.clear();
				it->second[j]->setPages( path, "" );
			}
		}
	}
}

// Send the response to the client
void	Server::sendResponse(Client *client, std::string response, int c_fd, std::string htmlFileName )
{
	int	bytesSent, toSend, alreadySent = 0, totalSent = 0;

	//if a part of respone is already sent, send from that point
	if ( client->sendAgain() == true )
	{
		alreadySent = client->getBytesSent();
		totalSent = alreadySent;
		client->setSendAgain( false );
	}
	toSend = response.size() - alreadySent;

	//create response which is sent back to client
	char	response_cstr[toSend];
	for ( int i = 0; i < toSend; i++, alreadySent++ ) {
		response_cstr[i] = response[alreadySent];
	}
	bytesSent = send( c_fd, response_cstr, toSend, 0 );
	if ( bytesSent == -1 )
	{
		this->_responseHeader.clear();
		std::ifstream ifs( "response/responseCGI" );
		if ( ifs.good() )
			ifs.close();
		closeConnection( client );
		removeResponseFiles();
		resetPages();
		std::cout << "error sending data to client.." << std::endl;
		if ( htmlFileName == "response/responseCGI" )
			exit ( 0 );
		return ;
	}
	totalSent = totalSent + bytesSent;
	if ( bytesSent < toSend )
	{
		client->setSendAgain( true );
		client->setBytesSent( totalSent );
	}
	client->update_client_timestamp();
	std::cout << "\n\033[32m\033[1m" << "RESPONDED:\n\033[0m\033[32m" << std::endl << response << "\033[0m" << std::endl;
	return ;
}

// Build the header for the response file
int	Server::buildHeaderResponse( Client *client, std::ifstream &htmlFile, std::string htmlFileName )
{
	int	c_fd = client->getConnectionFD();

	std::string str;
	htmlFile.seekg( 0, std::ios::end );   
	str.reserve( htmlFile.tellg() );
	htmlFile.clear();
	htmlFile.seekg( 0, std::ios::beg );
	str.assign( ( std::istreambuf_iterator<char> ( htmlFile ) ), std::istreambuf_iterator<char>() );

	std::string len = std::to_string( str.size() );
	if ( BinaryFile( str ) == true )
		str = "Content-Type: application/octet-stream\r\n\r\n" + str;
	else if ( str.size() >= 16 && str.substr( 0, 16 ).find( "<!DOCTYPE html>") != std::string::npos )
		str = "Content-Type: text/html\r\n\r\n" + str;
	else
		str = "Content-Type: text/plain\r\n\r\n" + str;
	str = "Content-Length: " + len + "\r\n" + str;
	str =  this->_responseHeader + "\r\n" + str;

	//send the response
	this->sendResponse(client, str, c_fd, htmlFileName );
	
	//clear and remove header/files
	this->_responseHeader.clear();
	htmlFile.close();
	std::ifstream ifs( "response/responseCGI" );
	if ( ifs.good() )
		ifs.close();
	removeResponseFiles();
	resetPages();
	if ( htmlFileName == "response/responseCGI" )
		exit ( 0 );
	if ( client->bodyTooLarge() == true )
	{
		closeConnection( client );
		return CLIENT_REMOVED;
	}
    return 0;
}

// configure the right response message
int	Server::configureResponseToClient( Client *client )
{
	Config			*config = this->_sockets[client->getSockNum()]->getConfig( client->getLocation(), client->getHost(), client );
    std::string     htmlFileName;
    std::ifstream   htmlFile;
    std::ofstream 	ofs;

	// Get the correct response file
    htmlFileName = this->getHtmlFile( client );
	if ( htmlFileName == "DO NOTHING" )
		return 0;
    if ( !htmlFileName.size() )
        htmlFileName = config->errorPageDir + "500.html";
	if (htmlFileName == "DO NOTHING")
		return 0;
	htmlFileName = this->getErrorPage( htmlFileName, config );
    htmlFile.open( htmlFileName, std::ios::in | std::ios::binary );
	// Response file doesn't exist
    if ( !htmlFile.is_open() )
    {
		std::cout << "cant open filename:" << htmlFileName << std::endl;
    	this->_responseHeader = "HTTP/1.1 500 Error";
		htmlFileName = config->errorPageDir + "500.html";
    	htmlFile.open( config->errorPageDir + "500.html", std::ios::in | std::ios::binary );
		// Error file doesn't exist, as a last resort server creates its own 500 error return
		if ( !htmlFile.is_open() )
		{
			htmlFile.open( createResponseHtml() );
			if ( !htmlFile.is_open() )
			{
				std::cout << "couldnt create a response.." << std::endl;
				closeConnection( client );
				return 0;
			}
		}
    }
	return this->buildHeaderResponse( client, htmlFile, htmlFileName );
}
