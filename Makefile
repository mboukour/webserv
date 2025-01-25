CC = c++
NAME = Webserv
SRC = Webserv.cpp Parser/Parser.cpp Parser/ServerFactory/ServerFactory.cpp  Server/Server.cpp Server/ABlock/ABlock.cpp Server/Location/Location.cpp Server/ServerManager/ServerManager.cpp Debug/Debug.cpp
EXCEPTIONS = Exceptions/UnknownMethod/UnknownMethod.cpp
HTTP_REQUEST = HttpRequest/HttpRequest.cpp
SRC += $(EXCEPTIONS) $(HTTP_REQUEST)
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