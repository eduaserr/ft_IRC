#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <string>
#include "Parser.hpp"

class	Client;
class	Server;

class	Command
{
	private:
		Command(const Command &other);
		Command &operator=(const Command &other);

	public:
		virtual	~Command(){}
		virtual void execute(Server& server, Client& client, const IrcMessage& message) = 0;
};

/*
PassCommand
NickCommand
UserCommand
JoinCommand
PrivmsgCommand
PartCommand
QuitCommand
TopicCommand
InviteCommand
KickCommand
ModeCommand*/
#endif