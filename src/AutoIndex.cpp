#include "../inc/Server.hpp"

// create an automatically generated index for the requested directory
std::string Server::createAutoIndex( std::string root, std::string folder )
{
    std::ofstream   autoindexFile;
    DIR             *directory;

    // std::cout << folder[folder.size() - 1] << std::endl;
    if (folder[folder.size() - 1] != '/')
        folder += "/";
	std::string		page = root + folder;

    autoindexFile.open( "response/autoindex.html", std::ofstream::out | std::ofstream::trunc );
    if ( !autoindexFile.is_open() )
    {
        printerror( "failed to create autoindex: " );
        return root + "/pages/errorpages/404.html";
    }
    directory = opendir( page.c_str() );
    if ( !directory )
    {
        printerror( "can not open directory(createAutoIndex): " );
        return root + "/pages/errorpages/404.html";
    }

    // content of the autoindex file
    autoindexFile <<
    "<!DOCTYPE html>\n\
    <html lang=\"en\">\n\
    <head>\n\
    <title>Webserv</title>\n\
    <link rel=\"icon\" href=\"/images/favicon-32x32.ico\">\
    </head>\n\
    <body\">\n\
    <h1>" + folder + "</h1><br>" << std::endl;

    // find all files and directories in the requested direcotry
    for ( struct dirent *dirEntry = readdir( directory ); dirEntry; dirEntry = readdir( directory ) )
    {
        std::string link = std::string( dirEntry->d_name );
        if ( link != "." && link != ".." )
        {
            std::string	temp = page + link;
			DIR* 		tempDir = opendir( temp.c_str() );

            // add a file to the autoindex
			if ( !tempDir )
                autoindexFile << "<a href=\"" << folder + link << "\">/" << link << "</a><br>" << std::endl;
            // add a directory to the autoindex
            else
            {
                autoindexFile << "<a href=\"" << folder + link << "/\">/" << link << "/</a><br>" << std::endl;
                closedir( tempDir );
            }
        }
    }
    autoindexFile <<\
    "</body>\n\
    </html>" << std::endl;
	closedir( directory );
    return "response/autoindex.html";
}
