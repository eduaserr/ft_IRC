#include "../../include/commands/PartCommand.hpp"

PartCommand::PartCommand()
{
}

PartCommand::~PartCommand()
{
}

void PartCommand::execute(Server &server, Client &client, const Message &message)
{
	(void)server;
	(void)client;
	(void)message;
}