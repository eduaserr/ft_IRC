#ifndef COMMANDREGISTRY_HPP
# define COMMANDREGISTRY_HPP

#include "ICommand.hpp"
#include "Message.hpp"
#include <map>
#include <string>

class Server;
class Client;

class CommandRegistry
{
	private:
		CommandRegistry(const CommandRegistry &other);
		CommandRegistry &operator=(const CommandRegistry &other);

		std::map<std::string, ICommand *> _commands;

		std::string _normalize(const std::string &command) const;

	public:
		CommandRegistry();
		~CommandRegistry();

		bool registerCommand(const std::string &command, ICommand *handler);
		bool hasCommand(const std::string &command) const;
		ICommand *findCommand(const std::string &command) const;
		bool dispatch(Server &server, Client &client, const Message &message) const;
};

#endif