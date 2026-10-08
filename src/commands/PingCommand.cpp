#include "../../include/commands/PingCommand.hpp"

#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"

PingCommand::PingCommand()
{
}

PingCommand::~PingCommand()
{
}

void PingCommand::execute(Server &server, Client &client,
	const Message &message)
{
	(void)server;
	client.appendOutput(Replies::pong("ircserv", message.getTrailing()));
}
