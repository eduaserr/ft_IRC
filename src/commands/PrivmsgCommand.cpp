#include "../../include/commands/PrivmsgCommand.hpp"
#include "../../include/Channel.hpp"
#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include "../../include/Server.hpp"
#include <vector>

PrivmsgCommand::PrivmsgCommand()
{
}

PrivmsgCommand::~PrivmsgCommand()
{
}

void PrivmsgCommand::execute(Server &server, Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (params.empty())
	{
		client.appendOutput(Replies::numeric("ircserv", "461",
			client.getNickname(), "PRIVMSG"));
		return ;
	}
	if (!message.hasTrailing() || message.getTrailing().empty())
	{
		client.appendOutput(Replies::numeric("ircserv", "412",
			client.getNickname(), "No text to send"));
		return ;
	}
	const std::string &target = params[0];
	std::string reply = Replies::privmsg(client.getPrefix(), target,
		message.getTrailing());
	if (target[0] == '#' || target[0] == '&')
	{
		Channel *channel = server.getChannel(target);
		if (channel == NULL)
		{
			client.appendOutput(Replies::numeric("ircserv", "403",
				client.getNickname(), "No such channel: " + target));
			return ;
		}
		if (!channel->isMember(&client))
		{
			client.appendOutput(Replies::numeric("ircserv", "404",
				client.getNickname(), target + " :Cannot send to channel"));
			return ;
		}
		server.broadcast(*channel, reply, &client);
		return ;
	}
	Client *recipient = server.getClient(target);
	if (recipient == NULL)
	{
		client.appendOutput(Replies::numeric("ircserv", "401",
			client.getNickname(), target + " :No such nick/channel"));
		return ;
	}
	server.sendToClient(recipient->getFd(), reply);
}