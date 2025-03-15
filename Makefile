CC = c++
NAME = Webserv
SRC = Webserv.cpp Parser/Parser.cpp Parser/ServerFactory/ServerFactory.cpp Server/Server.cpp \
      Server/ABlock/ABlock.cpp Server/Location/Location.cpp Server/ServerManager/ServerManager.cpp \
      Debug/Debug.cpp
EXCEPTIONS = Exceptions/HttpErrorException/HttpErrorException.cpp
HTTP = Http/AHttp/AHttp.cpp Http/HttpRequest/HttpRequest.cpp Http/HttpResponse/HttpResponse.cpp \
       Http/HttpResponse/Methods/handleDeleteRequest.cpp Http/HttpResponse/Methods/handleGetRequest.cpp \
       Http/HttpResponse/Methods/handlePostRequest.cpp
STATE = ConnectionState/ConnectionState.cpp
CGI = Cgi/Cgi.cpp
SESSION = Session/Login/Login.cpp
UTILS = Utils/Logger/Logger.cpp
SRC += $(EXCEPTIONS) $(HTTP) $(CGI) $(SESSION) $(UTILS) $(STATE)
OBJ = $(SRC:.cpp=.o)
FLAGS = -Wall -Wextra -Werror -std=c++98 -Wsign-conversion -Werror
HEADERS = Webserv.hpp Parser/Parser.hpp Parser/ServerFactory/ServerFactory.hpp Debug/Debug.hpp \
		  Server/Server.hpp Server/ABlock/ABlock.hpp Server/Location/Location.hpp Server/ServerManager/ServerManager.hpp \
		  Http/AHttp/AHttp.hpp Http/HttpRequest/HttpRequest.hpp Http/HttpResponse/HttpResponse.hpp \
		  Exceptions/HttpErrorException/HttpErrorException.hpp \
		  Cgi/Cgi.hpp
all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $^ -o $@

%.o: %.cpp $(HEADERS)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re:fclean all
