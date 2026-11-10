#include "../../include/commands/TopicCommand.hpp"
#include "../../include/Channel.hpp"
#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include "../../include/Server.hpp"
#include <vector>

TopicCommand::TopicCommand()
{
}

TopicCommand::~TopicCommand()
{
}

void TopicCommand::execute(Server &server, Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (params.empty())
	{
		client.appendOutput(Replies::numeric("ircserv", "461",
			client.getNickname(), "TOPIC"));
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
	if (!message.hasTrailing())
	{
		if (channel->getTopic().empty())
			client.appendOutput(Replies::noTopic("ircserv", client.getNickname(), params[0]));
		else
			client.appendOutput(Replies::topicReply("ircserv", client.getNickname(),
				params[0], channel->getTopic()));
		return ;
	}
	if (channel->isTopicRestricted() && !channel->isOperator(&client))
	{
		client.appendOutput(Replies::numeric("ircserv", "482",
			client.getNickname(), params[0] + " :You're not channel operator"));
		return ;
	}
	channel->setTopic(message.getTrailing());
	server.broadcast(*channel, Replies::topic(client.getPrefix(), params[0],
		message.getTrailing()));
}