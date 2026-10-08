#include "../include/CommandRegistry.hpp"

#include <cctype>

CommandRegistry::CommandRegistry() : _commands()
{
}

CommandRegistry::~CommandRegistry()
{
	for (std::map<std::string, ICommand *>::iterator it = _commands.begin();
		it != _commands.end(); ++it)
		delete it->second;
}

bool CommandRegistry::registerCommand(const std::string &command, ICommand *handler)
{
	std::string key = _normalize(command);

	if (handler == NULL || key.empty() || hasCommand(key))
	{
		delete handler;
		return false;
	}
	_commands[key] = handler;
	return true;
}

bool CommandRegistry::hasCommand(const std::string &command) const
{
	return _commands.find(_normalize(command)) != _commands.end();
}

ICommand *CommandRegistry::findCommand(const std::string &command) const
{
	std::map<std::string, ICommand *>::const_iterator it;

	it = _commands.find(_normalize(command));
	if (it == _commands.end())
		return NULL;
	return it->second;
}

bool CommandRegistry::dispatch(Server &server, Client &client,
	const Message &message) const
{
	ICommand *handler = findCommand(message.getCommand());

	if (handler == NULL)
		return false;
	handler->execute(server, client, message);
	return true;
}

std::string CommandRegistry::_normalize(const std::string &command) const
{
	std::string result = command;

	for (std::size_t i = 0; i < result.size(); ++i)
		result[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[i])));
	return result;
}
