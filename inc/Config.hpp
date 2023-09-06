#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "Server.hpp"

class Config;

typedef  void ( Config::*ConfigMemFn )( std::string );
#define CALL_MEMBER_FN( ptrToMember )( this->*( ptrToMember ) )

class Config
{
		Config() {};
    public:
        Config( std::string, std::string );
        ~Config();
		Config( const Config& );
    	Config  &operator=( const Config& );

		std::string					servername;
		std::string                 root;
		bool						autoindex;
		std::vector<std::string>	methods;
		std::string					index;
		std::string					cgi;
		size_t						maxClientBodySize;
		std::string					extension;
		std::string					uploadDir;
		std::string					errorPageDir;
		std::string					location;

		std::map< std::string, std::string >	pages; /* name - location */	
		std::map< std::string, std::string >	redirects; /* name - redirect_location */
		std::map< std::string, std::string >	errorPages; /* error - page */

		void	setConfig( std::string );
		void	setRedirects( std::string, std::string );
		int		setPages( std::string , std::string );
		void	setServerName( std::string );
		void	setRoot( std::string );
		void	setErrorPages( std::string );
		void	setAutoIndex( std::string );
		void	setIndex( std::string );
		void	setCGI( std::string );
		void	setMaxBodySize( std::string );
		void	setUploadDir( std::string );
		void	setExtension( std::string );
		void	setMethods( std::string );
		void	setLocation( std::string );

};

#endif
