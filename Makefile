CC = c++
NAME = Webserv
SRC = Webserv.cpp Parser/Parser.cpp Parser/LogicalValidator/LogicalValidator.cpp
OBJ = $(SRC:.cpp=.o)
FLAGS = -Wall -Wextra -Werror
HEADERS = Webserv.hpp Parser/Parser.hpp Parser/LogicalValidator/LogicalValidator.hpp
all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $^ -o $@

%.o: %.cpp $(HEADERS)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)