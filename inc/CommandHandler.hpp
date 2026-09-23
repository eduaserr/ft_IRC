#ifndef COMMAND_HANDLER_HPP
# define COMMAND_HANDLER_HPP

#include <map>
#include <string>

class Command;
class Client;
class Server;
struct IrcMessage;

class CommandHandler
{
private:
	std::map<std::string, Command *> _commands;

	CommandHandler(const CommandHandler &other);
	CommandHandler &operator=(const CommandHandler &other);

public:
	CommandHandler();
	~CommandHandler();

	void handle(Server& server, Client& client, const IrcMessage& message);
};

#endif