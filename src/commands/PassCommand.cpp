#include "../../include/commands/PassCommand.hpp"

#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include "../../include/Server.hpp"

PassCommand::PassCommand()
{
}

PassCommand::~PassCommand()
{
}

void PassCommand::execute(Server &server, Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (client.isRegistered())
	{
		client.appendOutput(Replies::numeric("ircserv", "462", client.getNickname(), "Already registered"));
		return ;
	}
	if (params.size() != 1)
	{
		client.appendOutput(Replies::numeric("ircserv", "461", client.getNickname(), "PASS"));
		return ;
	}
	if (params[0] != server.getPassword())
	{
		client.appendOutput(Replies::numeric("ircserv", "464", client.getNickname(), "Password incorrect"));
		return ;
	}

	client.setPasswordAccepted(true);
	server.tryRegister(client);
}
