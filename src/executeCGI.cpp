#include "../inc/Server.hpp"

static void	removeBoundaries( std::string & body, std::string boundary, size_t begin )
{
	size_t	start, end;

	start = body.find( "--" + boundary, begin );
	while ( start != std::string::npos )
	{
		end = body.find( "\r\n\r\n", start ) + 4;
		std::string substr = body.substr( start, end - start );
		if ( substr.find( "Content-Type: " ) != std::string::npos )
		{
			body.erase( start, end - start );
			start = body.find( "--" + boundary, start );
		}
		else
			start = body.find( "--" + boundary, start + 1 );
	}
}

std::vector<std::string>	readFile( std::string header, std::string body )
{
    size_t                      pos;
    std::vector<std::string>    vars;
    std::string                 boundary, filename, fileContent;

    pos = header.find( "boundary=" );
    if ( pos == std::string::npos )
    {
        printerror( "request has no boundary: " );
        return ( vars );
    }
    boundary = header.substr( header.find( "=", pos ) + 1, header.find( "\r\n", pos ) - ( header.find( "=", pos ) + 1 ) );
    pos = body.find( "--" + boundary );
    pos = body.find( "filename=", pos ) + 10;
    filename = body.substr( pos, body.find( "\r\n", pos ) - ( pos + 1 ) );
	if ( filename.empty() )
		return ( vars );
    pos = body.find( "\r\n\r\n", pos ) + 4;
	removeBoundaries( body, boundary, pos );
    fileContent = body.substr( pos, body.find( "--" + boundary, pos ) - ( pos + 2 ) );
    vars.push_back( filename );
    vars.push_back( fileContent );
    return ( vars );
}

// set the environment for CGI
std::map<std::string, std::string>   setupEnv( std::string page, int port, std::string path, std::string root, std::string body, std::string header, std::string uploaddir, std::string method )
{
    std::map<std::string, std::string>	env;
    std::vector<std::string>			vars;
    size_t								pos;
    std::string							contentType;

    // find the content type and set the environment accordingly
    if ( method == "GET" )
    {
        env["FILE_NAME"] = "form.log";
        env["FILE_BODY"] = body;
        env["HTTP_HOST"] =  "localhost:" + std::to_string( port );
        env["REQUEST_URI"] = page;
        env["REMOTE_PORT"] = std::to_string( port );
        env["REQUEST_METHOD"] = method;
        env["SERVER_PORT"] = std::to_string( port );
        env["UPLOAD_DIR"] = path + "/" + root + uploaddir;
        return (env);
    }
    pos = header.find( "Content-Type: " );
    if ( pos == std::string::npos)
    {
        printerror( "no Content-Type: " );
        return ( env );
    }
    if (header.find( "\r\n", pos ) < header.find( ";", pos ))
        contentType = header.substr( header.find( " ", pos ) + 1, header.find( "\r\n", pos ) - ( header.find( " ", pos ) + 1 ) );
    else
        contentType = header.substr( header.find( " ", pos ) + 1, header.find( ";", pos ) - ( header.find( " ", pos ) + 1 ) );
    // content type is a form
    if ( contentType == "application/x-www-form-urlencoded" )
    {
        env["FILE_NAME"] = "form.log";
        env["FILE_BODY"] = body;
    }
    // content type is a file
    else if ( contentType == "text/plain" )
    {
        if (body.size() > 10)
            env["FILE_NAME"] = body.substr(0, 10);
        else
            env["FILE_NAME"] = body;
        env["FILE_BODY"] = body;
        if (body.find("=") != std::string::npos)
        {
            env["FILE_NAME"] = page + "/" + body.substr(0, body.find("="));
            env["FILE_BODY"] = body.substr(body.find("=") + 1, body.size() - (body.find("=") + 1));
        }
        env["BODY_LEN"] = std::to_string( env["FILE_BODY"].size() );
    }
    //content type is a file with a boundary
    else if ( contentType == "multipart/form-data")
    {
        vars = readFile( header, body );
        if ( vars.empty() )
            return ( env );
        env["FILE_NAME"] = vars[0];
        env["FILE_BODY"] = vars[1];
		env["BODY_LEN"] = std::to_string( vars[1].size() );
    }
    else
    {
        printerror( "invalid Content-Type: " );
        return ( env );
    }
    env["HTTP_HOST"] =  "localhost:" + std::to_string( port );
    env["REQUEST_URI"] = page;
    env["REMOTE_PORT"] = std::to_string( port );
    env["REQUEST_METHOD"] = method;
    env["SERVER_PORT"] = std::to_string( port );
    env["UPLOAD_DIR"] = path + "/" + root + uploaddir;
    return ( env );
}

// execute the CGI
int	executeCGI( std::string page, int port, std::string path, std::string root, std::string body, std::string header, std::string uploaddir, std::string method )
{
    std::map<std::string, std::string>  env;
    pid_t		                        pid, pid2, timeout_pid;
    std::string	                        pathCGI, temp;
	std::ofstream						ofs;
    int                                 status;

    if (body.size() >= MAX_ARGLEN_EXECVE)               // returns when the body size is too large for execve to handle
        return 1;
    // setup the environmental variables for execve
	env = setupEnv( page, port, path, root, body, header, uploaddir, method );
    if ( !env.size() )
        return printerror( "failed setting up the environment: " );
    pathCGI = path + page;
    pid = fork();
    if ( pid == -1 )
        return printerror( "fork failed: " );
    if ( !pid )                                         // (child 1) executes the script while the server continues
    {
        timeout_pid = fork();
        if ( !timeout_pid )                             // (child 2) exits after ~5seconds
        {
            sleep( 5 );
            exit( 0 );
        }
        pid2 = fork();
        if ( pid2 == -1 )
            exit( printerror( "fork failed: " ) );
        if ( !pid2 )                                    // (child 3) executes the script
        {
            char    **c_env = new char*[env.size() + 1];
            int     i = 0;
            for ( std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); it++ )
            {
                temp = it->first + "=" + it->second;
                c_env[i] = new char[temp.size() + 1];
	        	for ( size_t j = 0; j < temp.size(); ++j ) {
	        		c_env[i][j] = temp[j];
	        	}
	        	c_env[i][temp.size()] = '\0';
                i++;
            }
            c_env[i] = NULL;
            ofs.open("response/responseCGI");
            ofs.close();
            int fd = open("response/responseCGI", O_WRONLY);
            if ( !fd )
                exit (printerror("failed to open file: "));
            dup2(fd, 1);
            execve(pathCGI.c_str(), NULL, c_env);
            exit( printerror( "execve failed: " ) );
        }
        else                                            // (child 1) waits for child 2 or 3 to exit
        {
            pid_t exited_pid = wait(&status);
            if (exited_pid == timeout_pid)              // child 2 exits first, so child 1 terminates child 3 and returns 1
            {
				std::cout << "script timed out: possible infinite loop\n";
                kill(pid2, SIGKILL);
                return 1;
            }
            kill(timeout_pid, SIGKILL);                 // child 3 exits first, script executed successfully, child 1 terminates child 2 and returns exitstatus of child 3
			if ( status == 65280 )
			{
				std::cout << "error in script\n";
				return 1;
			}
			return ( WEXITSTATUS(status) );
        }
    }
    return -2;
}
