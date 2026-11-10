#include "../../include/commands/WhoCommand.hpp"
#include "../../include/Channel.hpp"
#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include "../../include/Server.hpp"
#include <vector>

WhoCommand::WhoCommand()
{
}

WhoCommand::~WhoCommand()
{
}

void WhoCommand::execute(Server &server, Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();
	std::string mask;
	std::vector<Client *> members;

	if (params.empty())
	{
		client.appendOutput(Replies::numeric("ircserv", "461",
			client.getNickname(), "WHO"));
		return ;
	}
	mask = params[0];
	Channel *channel = server.getChannel(mask);
	if (channel != NULL)
		members = channel->getMembers();
	else
	{
		Client *target = server.getClient(mask);
		if (target != NULL)
			members.push_back(target);
	}
	for (std::size_t i = 0; i < members.size(); ++i)
	{
		client.appendOutput(Replies::whoReply("ircserv", client.getNickname(),
			mask, members[i]->getUsername(), members[i]->getHost(), "ircserv",
			members[i]->getNickname(), channel != NULL && channel->isOperator(members[i]),
			members[i]->getRealname()));
	}
	client.appendOutput(Replies::whoEnd("ircserv", client.getNickname(), mask));
}
