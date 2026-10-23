NAME		:= ircserv

CXX			:= c++
CXXFLAGS	:= -Wall -Wextra -Werror -std=c++98 -Iinclude -g -O0

SRCS		:= main.cpp \
			   src/Server.cpp \
			   src/Client.cpp \
			   src/Channel.cpp \
			   src/Message.cpp \
			   src/Parser.cpp \
			   src/CommandRegistry.cpp \
			   src/ICommand.cpp \
			   src/commands/PingCommand.cpp \
			   src/commands/CapCommand.cpp \
			   src/commands/PassCommand.cpp \
			   src/commands/NickCommand.cpp \
			   src/commands/UserCommand.cpp \
			   src/commands/InviteCommand.cpp \
			   src/commands/JoinCommand.cpp \
			   src/commands/KickCommand.cpp \
			   src/commands/PartCommand.cpp \
			   src/commands/PrivmsgCommand.cpp \
			   src/commands/QuitCommand.cpp \
			   src/commands/TopicCommand.cpp \
			   src/commands/ModeCommand.cpp \
			   src/Replies.cpp \
			   src/Config.cpp

OBJS		:= $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
