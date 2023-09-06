NAME = webserver

CC = g++

CFLAGS = -Wall -Werror -Wextra -std=c++98 #-fsanitize=address

SRC_DIR =	src/
DIRS = obj/ response/ forms/ logs/
OBJ_DIR =	obj/
RESPONSE_DIR =	response/
UPLOAD_DIR = uploads/
LOG_DIR = logs/

HEADER_FILES = inc/Server.hpp inc/Socket.hpp inc/Client.hpp inc/Config.hpp

SRC =	src/webserv.cpp\
		src/Socket.cpp\
		src/Server.cpp\
		src/HandleEvents.cpp\
		src/utils.cpp\
		src/Client.cpp\
		src/ParseRequests.cpp\
		src/Config.cpp\
		src/AutoIndex.cpp\
		src/Methods.cpp\
		src/executeCGI.cpp\
		src/Connections.cpp\

OBJ = $(SRC:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)

all: $(DIRS)
	@$(MAKE) $(NAME) -j4

$(NAME): $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o $(NAME)

$(OBJ): $(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(HEADER_FILES)
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

$(DIRS):
	@mkdir -vp $(DIRS)

clean:
	@rm -f $(OBJ)
	@rm -f uploads/*
	@rm -f logs/*
	@rm -f response/*

fclean: clean
	@rm -f $(NAME)
	@rm -rf $(RESPONSE_DIR)
	@rm -rf $(UPLOAD_DIR)
	@rm -rf $(LOG_DIR)

re: fclean all

.PHONY: all clean fclean re
