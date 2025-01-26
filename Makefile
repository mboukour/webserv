CC = c++
NAME = Webserv
SRC = Webserv.cpp Parser/Parser.cpp Parser/ServerFactory/ServerFactory.cpp  Server/Server.cpp Server/ABlock/ABlock.cpp Server/Location/Location.cpp Server/ServerManager/ServerManager.cpp Debug/Debug.cpp
EXCEPTIONS = Exceptions/UnknownMethodException/UnknownMethodException.cpp Exceptions/HttpRequestParseException/HttpRequestParseException.cpp Exceptions/NotImplementedException/NotImplementedException.cpp Exceptions/PayloadTooLargeException/PayloadTooLargeException.cpp Exceptions/MethodNotAllowedException/MethodNotAllowedException.cpp Exceptions/NotFoundException/NotFoundException.cpp
HTTP = Http/AHttp/AHttp.cpp Http/HttpRequest/HttpRequest.cpp Http/HttpResponse/HttpResponse.cpp Http/HttpResponse/HttpResponseErrorMaker/HttpResponseErrorMaker.cpp
SRC += $(EXCEPTIONS) $(HTTP)
OBJ = $(SRC:.cpp=.o)
FLAGS = -Wall -Wextra -Werror  -std=c++98
HEADERS = Webserv.hpp Parser/Parser.hpp Parser/ServerFactory/ServerFactory.hpp Debug/Debug.hpp
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