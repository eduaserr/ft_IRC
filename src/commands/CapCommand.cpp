#include "../../include/commands/CapCommand.hpp"

#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include <cctype>
#include <vector>

CapCommand::CapCommand()
{
}

CapCommand::~CapCommand()
{
}

void CapCommand::execute(Server &server, Client &client, const Message &message)
{
	(void)server;
	const std::vector<std::string> &params = message.getParams();
	std::string	subcmd;

	if (params.empty())
	{
		client.appendOutput(Replies::numeric("ircserv", "461",
			client.getNickname(), "CAP"));
		return ;
	}
	subcmd = params[0];
	for (std::size_t i = 0; i < subcmd.size(); ++i) {
		subcmd[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(subcmd[i])));
	}

	if (subcmd == "LS")
	{
		client.appendOutput(Replies::capLs("*", ""));
		return ;
	}
	if (subcmd == "REQ")
	{
		client.appendOutput(Replies::capNak("*", message.getTrailing()));
		return ;
	}
	if (subcmd == "LIST")
	{
		client.appendOutput(Replies::capList("*", ""));
		return ;
	}
	if (subcmd == "END")
		return ;

	client.appendOutput(Replies::capNak("*", subcmd));
}