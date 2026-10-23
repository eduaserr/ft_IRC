#include "../../include/commands/JoinCommand.hpp"

JoinCommand::JoinCommand()
{
}

JoinCommand::~JoinCommand()
{
}

void JoinCommand::execute(Server &server, Client &client, const Message &message)
{
	(void)server;
	(void)client;
	(void)message;
}