#include "../../include/commands/QuitCommand.hpp"

QuitCommand::QuitCommand()
{
}

QuitCommand::~QuitCommand()
{
}

void QuitCommand::execute(Server &server, Client &client, const Message &message)
{
	(void)server;
	(void)client;
	(void)message;
}