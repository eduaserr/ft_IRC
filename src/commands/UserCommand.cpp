#include "../../include/commands/UserCommand.hpp"

#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include "../../include/Server.hpp"

UserCommand::UserCommand()
{
}

UserCommand::~UserCommand()
{
}

void UserCommand::execute(Server &server, Client &client,
	const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (client.isRegistered())
	{
		client.appendOutput(Replies::numeric("ircserv", "462",
			client.getNickname(), "Already registered"));
		return ;
	}
	if (params.size() < 3 || !message.hasTrailing())
	{
		client.appendOutput(Replies::numeric("ircserv", "461",
			client.getNickname(), "USER"));
		return ;
	}
	client.setUsername(params[0]);
	client.setRealname(message.getTrailing());
	server.tryRegister(client);
}