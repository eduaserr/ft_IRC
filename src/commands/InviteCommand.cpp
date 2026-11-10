#include "../../include/commands/InviteCommand.hpp"
#include "../../include/Channel.hpp"
#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include "../../include/Server.hpp"
#include <vector>

InviteCommand::InviteCommand()
{
}

InviteCommand::~InviteCommand()
{
}

void InviteCommand::execute(Server &server, Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (params.size() < 2)
	{
		client.appendOutput(Replies::numeric("ircserv", "461",
			client.getNickname(), "INVITE"));
		return ;
	}
	Client *target = server.getClient(params[0]);
	if (target == NULL)
	{
		client.appendOutput(Replies::numeric("ircserv", "401",
			client.getNickname(), params[0] + " :No such nick/channel"));
		return ;
	}
	Channel *channel = server.getChannel(params[1]);
	if (channel == NULL)
	{
		client.appendOutput(Replies::numeric("ircserv", "403",
			client.getNickname(), params[1] + " :No such channel"));
		return ;
	}
	if (!channel->isMember(&client))
	{
		client.appendOutput(Replies::numeric("ircserv", "442",
			client.getNickname(), params[1] + " :You're not on that channel"));
		return ;
	}
	if (!channel->isOperator(&client))
	{
		client.appendOutput(Replies::numeric("ircserv", "482",
			client.getNickname(), params[1] + " :You're not channel operator"));
		return ;
	}
	if (channel->isMember(target))
	{
		client.appendOutput(Replies::numeric("ircserv", "443",
			client.getNickname(), params[0] + " " + params[1]
			+ " :is already on channel"));
		return ;
	}
	channel->addInvite(target);
	server.sendToClient(target->getFd(), Replies::invite(client.getPrefix(),
		params[0], params[1]));
}