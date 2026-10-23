#include "../../include/commands/KickCommand.hpp"

KickCommand::KickCommand()
{
}

KickCommand::~KickCommand()
{
}

void KickCommand::execute(Server &server, Client &client, const Message &message)
{
	(void)server;
	(void)client;
	(void)message;
}