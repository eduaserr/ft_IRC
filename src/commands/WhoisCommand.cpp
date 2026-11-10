#include "../../include/commands/WhoisCommand.hpp"
#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include "../../include/Server.hpp"
#include <vector>

WhoisCommand::WhoisCommand()
{
}

WhoisCommand::~WhoisCommand()
{
}

void WhoisCommand::execute(Server &server, Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();
	Client *target;
	const std::string &nickname = params.empty() ? std::string() : params[0];

	if (params.empty())
	{
		client.appendOutput(Replies::numeric("ircserv", "461",
			client.getNickname(), "WHOIS"));
		return ;
	}
	target = server.getClient(nickname);
	if (target == NULL)
	{
		client.appendOutput(Replies::numeric("ircserv", "401",
			client.getNickname(), nickname + " :No such nick/channel"));
		client.appendOutput(Replies::whoisEnd("ircserv", client.getNickname(),
			nickname));
		return ;
	}
	client.appendOutput(Replies::whoisUser("ircserv", client.getNickname(),
		target->getNickname(), target->getUsername(), target->getHost(),
		target->getRealname()));
	client.appendOutput(Replies::whoisServer("ircserv", client.getNickname(),
		target->getNickname(), "ircserv", "IRC server"));
	client.appendOutput(Replies::whoisEnd("ircserv", client.getNickname(),
		target->getNickname()));
}
