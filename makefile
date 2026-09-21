### COMPILATION ###
NAME	= ircserv
CC	= c++
CFLAGS	= -Wall -Wextra -Werror -std=c++98

MAKE	= make --no-print-directory
RM		= rm -f

### SRCS ###
INC		= inc/

	SRC		= main.cpp \
			  src/Server.cpp

### OBJS ###
OBJS	= $(SRC:.cpp=.o)

### RULES ###
all : $(NAME)

$(NAME): $(OBJS)
	@echo "loading IRC server..."
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "IRC server compiled successfully"

%.o : %.cpp
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "clearing IRC server...🧹"
	@$(RM) $(OBJS) main.o

fclean: clean
	@$(RM) $(NAME) main
	@echo "clearing IRC server executable"

re: fclean all

.PHONY: all clean fclean re