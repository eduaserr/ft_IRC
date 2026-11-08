#include "../../include/commands/JoinCommand.hpp"
#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include <cctype>
#include <iostream>
#include <vector>

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