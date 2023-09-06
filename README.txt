This project is a simple HTTP webserver in cpp which can listen to multiple ports
To use it, simple make and run it, it will use the default config.
It handles GET, DELETE and POST
Files can be uploaded and forms can be posted - it will do so (with or without) using CGI

In the config file you can set up server blocks. Each block should be within brackets
{
	[block]
}

Each block has a general serverconfig where you can configure the following:
{
	listen [port] [host]  					-> sets the port and host the server listens to
	errorPages [/some_directory/]			-> location of default error pages
	maxClientBodySize [some_number]			-> a maximum upload size (bytes)
	autoindex [on/off]						-> turns on/off directory listing (off by default)
	methods [GET+POST+DELETE] 				-> sets allowed methods
	root [some_directory]					-> location of root folder where resources are searched from
	directoryRequest [some_html_location]	-> standard page to return when a directory is requested
	cgi [some_directory]					-> location of cgi scripts
	extension [.some_extention]				-> sets allowed extension for cgi scripts
	redirect [to_some_location]				-> set page to redirect to
	uploadDir [/some_directory/]			-> location where uploaded files are stored
	location [requested location] {			-> setup route 
		...
	}
}

All configuration options except for listen can also be set for specific routes.
The redirect only functions for a specific route

example
{
	listen 8080

	location / {
		autoindex on
	}
	location /pages/abc.txt {
		methods GET+DELETE
	}
}

spaces between words, indent with TABS!
