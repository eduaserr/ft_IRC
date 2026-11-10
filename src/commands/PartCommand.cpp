#include "../../include/commands/PartCommand.hpp"
#include "../../include/Channel.hpp"
#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include "../../include/Server.hpp"
#include <vector>

PartCommand::PartCommand()
{
}

PartCommand::~PartCommand()
{
}

void PartCommand::execute(Server &server, Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (params.empty())
	{
		client.appendOutput(Replies::numeric("ircserv", "461",
			client.getNickname(), "PART"));
		return ;
	}
	Channel *channel = server.getChannel(params[0]);
	if (channel == NULL)
	{
		client.appendOutput(Replies::numeric("ircserv", "403",
			client.getNickname(), "No such channel: " + params[0]));
		return ;
	}
	if (!channel->isMember(&client))
	{
		client.appendOutput(Replies::numeric("ircserv", "442",
			client.getNickname(), params[0] + " :You're not on that channel"));
		return ;
	}
	std::string reason;
	if (message.hasTrailing())
		reason = message.getTrailing();
	server.broadcast(*channel, Replies::part(client.getPrefix(), params[0], reason));
	channel->removeMember(&client);
	if (channel->isEmpty())
		server.removeChannel(params[0]);
}