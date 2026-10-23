#include "../../include/commands/NickCommand.hpp"

#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include "../../include/Server.hpp"
#include <cctype>
#include <string>
#include <vector>

NickCommand::NickCommand()
{
}

NickCommand::~NickCommand()
{
}

void NickCommand::execute(Server &server, Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();
	std::string nickname;

	if (params.empty())
	{
		client.appendOutput(Replies::numeric("ircserv", "431",
			client.getNickname(), "No nickname given"));
		return ;
	}
	if (params.size() != 1)
	{
		client.appendOutput(Replies::numeric("ircserv", "432",
			client.getNickname(), "Erroneous nickname"));
		return ;
	}
	nickname = params[0];
	if (nickname.empty() || !std::isalpha(
		static_cast<unsigned char>(nickname[0])))
	{
		client.appendOutput(Replies::numeric("ircserv", "432",
			client.getNickname(), "Erroneous nickname"));
		return ;
	}
	for (std::size_t i = 1; i < nickname.size(); ++i)
	{
		if (!std::isalnum(static_cast<unsigned char>(nickname[i]))
			&& nickname[i] != '-' && nickname[i] != '_')
		{
			client.appendOutput(Replies::numeric("ircserv", "432",
				client.getNickname(), "Erroneous nickname"));
			return ;
		}
	}
	if (server.nicknameInUse(nickname, &client))
	{
		client.appendOutput(Replies::numeric("ircserv", "433",
			client.getNickname(), "Nickname is already in use"));
		return ;
	}
	client.setNickname(nickname);
	server.tryRegister(client);
}