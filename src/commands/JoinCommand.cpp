#include "../../include/commands/JoinCommand.hpp"
#include "../../include/Channel.hpp"
#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include "../../include/Server.hpp"
#include <string>
#include <vector>

JoinCommand::JoinCommand()
{
}

JoinCommand::~JoinCommand()
{
}

void JoinCommand::execute(Server &server, Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (params.empty())
	{
		client.appendOutput(Replies::numeric("ircserv", "461",
			client.getNickname(), "JOIN"));
		return ;
	}
	std::string channelName = params[0];
	if (channelName.empty() || (channelName[0] != '#'
		&& channelName[0] != '&'))
	{
		client.appendOutput(Replies::numeric("ircserv", "403",
			client.getNickname(), "No such channel: " + channelName));
		return ;
	}
	Channel *channel = server.getChannel(channelName);
	if (channel == NULL)
	{
		channel = server.createChannel(channelName);
		channel->addMember(&client);
		channel->addOperator(&client);
	}
	else if (channel->isMember(&client))
		return ;
	else
	{
		if (channel->isInviteOnly() && !channel->isInvited(&client))
		{
			client.appendOutput(Replies::numeric("ircserv", "473",
				client.getNickname(), channelName + " :Invite only channel"));
			return ;
		}
		if (!channel->getKey().empty()
			&& (params.size() < 2 || params[1] != channel->getKey()))
		{
			client.appendOutput(Replies::numeric("ircserv", "475",
				client.getNickname(), channelName + " :Cannot join channel (+k)"));
			return ;
		}
		if (channel->getUserLimit() > 0
			&& static_cast<int>(channel->getMemberCount())
			>= channel->getUserLimit())
		{
			client.appendOutput(Replies::numeric("ircserv", "471",
				client.getNickname(), channelName + " :Cannot join channel (+l)"));
			return ;
		}
		channel->addMember(&client);
		channel->removeInvite(&client);
	}

	server.broadcast(*channel, Replies::join(client.getPrefix(), channelName));
	if (channel->getTopic().empty())
		client.appendOutput(Replies::noTopic("ircserv", client.getNickname(),
			channelName));
	else
		client.appendOutput(Replies::topicReply("ircserv", client.getNickname(),
			channelName, channel->getTopic()));
	std::string names;
	const std::vector<Client *> &members = channel->getMembers();
	for (std::size_t i = 0; i < members.size(); ++i)
	{
		if (i != 0)
			names += " ";
		if (channel->isOperator(members[i]))
			names += "@";
		names += members[i]->getNickname();
	}
	client.appendOutput(Replies::namesReply("ircserv", client.getNickname(),
		channelName, names));
	client.appendOutput(Replies::endOfNames("ircserv", client.getNickname(),
		channelName));
}