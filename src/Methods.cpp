#include "../inc/Server.hpp"

std::string	Server::methodGET( Client *client, Config *config )
{
	int			sock_num = client->getSockNum();
	int			port = this->_sockets[sock_num]->port;
	std::string	page, query, index, location;

	location = client->getLocation();
	if (location.find("?") != std::string::npos)
	{
		query = location.substr(location.find("?") + 1, location.size() - (location.find("?") + 1));
		location = location.substr(0, location.find("?"));
	}
    page = this->_sockets[sock_num]->getLocationPage( location, client->getHost(), client );
    // respond to a GET request that requests a directory
	if ( page == "Directory" )
	{
        // responds the (if set) index page
		_responseHeader = "HTTP/1.1 200 OK";
		if ( config->index != "" )
			return ( config->root + config->index );
        // else search for existing index
        index = location + "index.html";
		if ( location.back() != '/' )
			index = location + "/index.html";
		page = this->_sockets[sock_num]->getLocationPage( index, client->getHost(), client );
		if ( page != "" )
		    return ( page );
        // if there was no index -> create an autoindex (if enabled)
        if ( config->autoindex )
            return ( this->createAutoIndex( config->root, location ) );
        _responseHeader = "HTTP/1.1 403 Forbidden";
        return ( config->errorPageDir + "403.html" );
	}
	if ( page != "" )
    {
		// execute the CGI on the requested file if it has the right extension
		if ( location.size() > config->extension.size() && location.substr( location.size() - config->extension.size(), location.size() - 1) == config->extension )
		{
			switch ( executeCGI( "/" + config->root + location, port, this->_path, config->root, query, client->getHeader(), config->uploadDir, "GET" ) )
			{
				case 0:
					_responseHeader = "HTTP/1.1 200 OK";
					return ( "response/responseCGI" );
				case 1:
					_responseHeader = "HTTP/1.1 500 Error";
					return ( config->errorPageDir + "500.html" );
				case -1:
					_responseHeader = "HTTP/1.1 400 Bad Request";
					return ( config->errorPageDir + "400.html" );
				case -2:
					return ( "DO NOTHING" );
			}
		}
    	_responseHeader = "HTTP/1.1 200 OK";
    	return ( page );
    }
    _responseHeader = "HTTP/1.1 404 Not Found";
    return ( config->errorPageDir + "404.html" );
}

std::string	Server::methodPOST( Client *client, Config *config )
{
	int							sock_num = client->getSockNum(), fileExtension = 0;
	int							port = this->_sockets[sock_num]->port;
	std::string					location = client->getLocation(), page = this->_sockets[sock_num]->getLocationPage( location, client->getHost(), client );
	std::string					header = client->getHeader(), body = client->getBody(), newFileName, newFileContent, index, contentType;
	std::ofstream				newFile;
	std::ifstream				checkIfOpen;
	std::vector<std::string>	vars;

	if ( client->bodyTooLarge() == true )
	{
		_responseHeader = "HTTP/1.1 413 Request Entity Too Large";
		return ( config->errorPageDir + "413.html" );
	}
	if ( BinaryFile( client->getBody() ) == true )
	{
		SaveBinaryFile( this->_path, client, config );
		_responseHeader = "HTTP/1.1 200 OK";
		return ( "response/responseBinaryUpload" );
	}
	if ( page != "" )
    {
		// execute the CGI on the requested file if it has the right extension
		if ( location.size() > config->extension.size() && location.substr( location.size() - config->extension.size(), location.size() - 1) == config->extension )
		{
			switch ( executeCGI( "/" + config->root + location, port, this->_path, config->root, client->getBody(), client->getHeader(), config->uploadDir, "POST" ) )
			{
				std::cout << "asdasd\n";
				case 0:
					_responseHeader = "HTTP/1.1 200 OK";
					return ( "response/responseCGI" );
				case 1:
					_responseHeader = "HTTP/1.1 500 Error";
					return ( config->errorPageDir + "500.html" );
				case -1:
					_responseHeader = "HTTP/1.1 400 Bad Request";
					return ( config->errorPageDir + "400.html" );
				case -2:
					return ( "DO NOTHING" );
			}
		}
		if (client->getHeader().find("\r\n", client->getHeader().find("Content-Type: ") + 14) < client->getHeader().find(";", client->getHeader().find("Content-Type: ") + 14))
        	contentType = client->getHeader().substr(client->getHeader().find("Content-Type: ") + 14, client->getHeader().find("\r\n", client->getHeader().find("Content-Type: ") + 14) - (client->getHeader().find("Content-Type: ") + 14));
    	else
        	contentType = client->getHeader().substr(client->getHeader().find("Content-Type: ") + 14, client->getHeader().find(";", client->getHeader().find("Content-Type: ") + 14) - (client->getHeader().find("Content-Type: ") + 14));
		// POST request is a form
		if (contentType == "application/x-www-form-urlencoded")
		{
			size_t	posMid = body.find("="), posStart = 0;

			_responseHeader = "HTTP/1.1 201 Created";
			// put the content of the form in the form.log
			newFile.open("forms/form.log", std::ios_base::app);
  			newFile << "Local date and time ...?\n";
			if (posMid == std::string::npos)
			{
				newFile << body << "\n\n";
				return ( config->errorPageDir + "201.html" );
			}
			posStart = body.find("&");
			if (posStart != std::string::npos)
				newFile << body.substr(0, posMid) << " = " << body.substr(posMid + 1, body.find("&") - (posMid + 1)) << "\n";
			while (body.find("&", body.find("&", posMid) + 1) != std::string::npos)
			{
				posStart = body.find("&", posMid) + 1;
				posMid = body.find("=", posStart);
				newFile << body.substr(posStart, posMid - posStart) << " = " << body.substr(posMid + 1, body.find("&", posMid) - (posMid + 1)) << "\n";
			}
			posStart = body.find("&", posMid) + 1;
			posMid = body.find("=", posStart);
			newFile << body.substr(posStart, posMid - posStart) << " = " << body.substr(posMid + 1, body.size() - (posMid + 1)) << "\n\n";
			return ( config->errorPageDir + "201.html" );
		}
		// create a new file with the uploaded content
		newFileName = location + "/" + body.substr(0, 10);
		if (location.back() == '/')
			newFileName = location + body.substr(0, 10);
		newFileContent = body;
		if (contentType == "multipart/form-data")
		{
			vars = readFile(client->getHeader(), body);
			if ( vars.empty() )
			{
				_responseHeader = "HTTP/1.1 400 Bad Request";
				return ( config->errorPageDir + "400.html" );
			}
			newFileName = location + config->uploadDir + vars[0];
			if (location.back() == '/')
				newFileName = location.substr(0, location.size() - 1) + config->uploadDir + vars[0];
			newFileContent = vars[1];
		}
		checkIfOpen.open(config->root + newFileName);
		while ( checkIfOpen.is_open() )
		{
			fileExtension++;
			checkIfOpen.close();
			checkIfOpen.open(config->root + newFileName + std::to_string(fileExtension));
		}
		if (fileExtension)
			newFileName += std::to_string(fileExtension);
		newFile.open(config->root + newFileName);
		if ( !newFile.is_open() )
		{
			_responseHeader = "HTTP/1.1 405 Method Not Allowed";
    		return ( config->errorPageDir + "405.html" );
		}
		newFile << newFileContent;
		newFile.close();
		_responseHeader = "HTTP/1.1 201 Created";
		return ( config->errorPageDir + "201.html" );
	}
	_responseHeader = "HTTP/1.1 404 Not Found";
    return ( config->errorPageDir + "404.html" );
}

std::string	Server::methodDELETE( Client *client, Config *config )
{
	std::string		page = this->_sockets[client->getSockNum()]->getLocationPage( client->getLocation(), client->getHost(), client );
    std::ifstream	file;

	// check if the requested file exists and delete it
	file.open( config->root + client->getLocation() );
	if ( file )
		remove( ( config->root + client->getLocation() ).c_str() );
	_responseHeader = "HTTP/1.1 200 OK";
	return ( config->root + "/index.html" );
}
