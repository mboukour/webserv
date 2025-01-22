CC = c++
NAME = Webserv
SRC = Webserv.cpp Parser/Parser.cpp
OBJ = $(SRC:.cpp=.o)
FLAGS = -Wall -Wextra -Werror
all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $^ -o $@

%.o: %.cpp Webserv.hpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)