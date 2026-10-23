#include "../../include/commands/TopicCommand.hpp"

TopicCommand::TopicCommand()
{
}

TopicCommand::~TopicCommand()
{
}

void TopicCommand::execute(Server &server, Client &client, const Message &message)
{
	(void)server;
	(void)client;
	(void)message;
}