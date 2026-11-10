#include "../../include/commands/ModeCommand.hpp"
#include "../../include/Channel.hpp"
#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include "../../include/Server.hpp"
#include <cstdlib>
#include <vector>

ModeCommand::ModeCommand()
{
}

ModeCommand::~ModeCommand()
{
}

void ModeCommand::execute(Server &server, Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (params.empty())
	{
		client.appendOutput(Replies::numeric("ircserv", "461",
			client.getNickname(), "MODE"));
		return ;
	}
	Channel *channel = server.getChannel(params[0]);
	if (channel == NULL)
	{
		client.appendOutput(Replies::numeric("ircserv", "403",
			client.getNickname(), params[0] + " :No such channel"));
		return ;
	}
	if (params.size() == 1)
	{
		std::string modes = "+";
		if (channel->isInviteOnly())
			modes += "i";
		if (channel->isTopicRestricted())
			modes += "t";
		if (!channel->getKey().empty())
			modes += "k";
		if (channel->getUserLimit() > 0)
			modes += "l";
		client.appendOutput(Replies::mode("ircserv", params[0], modes, ""));
		return ;
	}
	if (!channel->isMember(&client) || !channel->isOperator(&client))
	{
		client.appendOutput(Replies::numeric("ircserv", "482",
			client.getNickname(), params[0] + " :You're not channel operator"));
		return ;
	}
	const std::string &requestedModes = params[1];
	std::size_t parameterIndex = 2;
	bool adding = true;
	std::string appliedModes;
	std::string appliedParameters;
	bool outputAdding = true;
	for (std::size_t i = 0; i < requestedModes.size(); ++i)
	{
		char mode = requestedModes[i];
		if (mode == '+')
		{
			adding = true;
			continue ;
		}
		if (mode == '-')
		{
			adding = false;
			continue ;
		}
		if (mode != 'i' && mode != 't' && mode != 'k'
			&& mode != 'o' && mode != 'l')
		{
			client.appendOutput(Replies::numeric("ircserv", "472",
				client.getNickname(), std::string(1, mode)
				+ " :is unknown mode char to me"));
			return ;
		}
		if (mode == 'i')
			channel->setInviteOnly(adding);
		else if (mode == 't')
			channel->setTopicRestricted(adding);
		else if (mode == 'k')
		{
			if (adding)
			{
				if (parameterIndex >= params.size())
				{
					client.appendOutput(Replies::numeric("ircserv", "461",
						client.getNickname(), "MODE"));
					return ;
				}
				channel->setKey(params[parameterIndex]);
				if (!appliedParameters.empty())
					appliedParameters += " ";
				appliedParameters += params[parameterIndex++];
			}
			else
				channel->setKey("");
		}
		else if (mode == 'o')
		{
			if (parameterIndex >= params.size())
			{
				client.appendOutput(Replies::numeric("ircserv", "461",
					client.getNickname(), "MODE"));
				return ;
			}
			Client *target = server.getClient(params[parameterIndex]);
			if (target == NULL)
			{
				client.appendOutput(Replies::numeric("ircserv", "401",
					client.getNickname(), params[parameterIndex]
					+ " :No such nick/channel"));
				return ;
			}
			if (adding)
				channel->addOperator(target);
			else
				channel->removeOperator(target);
			if (!appliedParameters.empty())
				appliedParameters += " ";
			appliedParameters += params[parameterIndex++];
		}
		else if (mode == 'l')
		{
			if (adding)
			{
				if (parameterIndex >= params.size()
					|| std::atoi(params[parameterIndex].c_str()) <= 0)
				{
					client.appendOutput(Replies::numeric("ircserv", "461",
						client.getNickname(), "MODE"));
					return ;
				}
				channel->setUserLimit(std::atoi(params[parameterIndex] .c_str()));
				if (!appliedParameters.empty())
					appliedParameters += " ";
				appliedParameters += params[parameterIndex++];
			}
			else
				channel->setUserLimit(-1);
		}
		if (appliedModes.empty())
		{
			appliedModes += adding ? "+" : "-";
			outputAdding = adding;
		}
		else if (outputAdding != adding)
		{
			appliedModes += adding ? "+" : "-";
			outputAdding = adding;
		}
		appliedModes += mode;
	}
	if (!appliedModes.empty())
		server.broadcast(*channel, Replies::mode(client.getPrefix(), params[0],
			appliedModes, appliedParameters));
}