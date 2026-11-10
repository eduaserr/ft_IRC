#include "../../include/commands/KickCommand.hpp"
#include "../../include/Channel.hpp"
#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include "../../include/Server.hpp"
#include <vector>

KickCommand::KickCommand()
{
}

KickCommand::~KickCommand()
{
}

void KickCommand::execute(Server &server, Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (params.size() < 2)
	{
		client.appendOutput(Replies::numeric("ircserv", "461",
			client.getNickname(), "KICK"));
		return ;
	}
	Channel *channel = server.getChannel(params[0]);
	if (channel == NULL)
	{
		client.appendOutput(Replies::numeric("ircserv", "403",
			client.getNickname(), params[0] + " :No such channel"));
		return ;
	}
	if (!channel->isMember(&client))
	{
		client.appendOutput(Replies::numeric("ircserv", "442",
			client.getNickname(), params[0] + " :You're not on that channel"));
		return ;
	}
	if (!channel->isOperator(&client))
	{
		client.appendOutput(Replies::numeric("ircserv", "482",
			client.getNickname(), params[0] + " :You're not channel operator"));
		return ;
	}
	Client *target = server.getClient(params[1]);
	if (target == NULL)
	{
		client.appendOutput(Replies::numeric("ircserv", "401",
			client.getNickname(), params[1] + " :No such nick/channel"));
		return ;
	}
	if (!channel->isMember(target))
	{
		client.appendOutput(Replies::numeric("ircserv", "441",
			client.getNickname(), params[1] + " " + params[0]
			+ " :They aren't on that channel"));
		return ;
	}
	std::string reason = message.hasTrailing() ? message.getTrailing() : params[1];
	server.broadcast(*channel, Replies::kick(client.getPrefix(), params[0],
		params[1], reason));
	channel->removeMember(target);
	if (channel->isEmpty())
		server.removeChannel(params[0]);
}