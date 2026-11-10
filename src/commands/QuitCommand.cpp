#include "../../include/commands/QuitCommand.hpp"
#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Server.hpp"

QuitCommand::QuitCommand()
{
}

QuitCommand::~QuitCommand()
{
}

void QuitCommand::execute(Server &server, Client &client, const Message &message)
{
	std::string reason = "Quit";

	if (message.hasTrailing() && !message.getTrailing().empty())
		reason = message.getTrailing();
	server.disconnectClient(client, reason);
}