#ifndef INVITECOMMAND_HPP
# define INVITECOMMAND_HPP

#include "ICommand.hpp"

class InviteCommand : public ICommand
{
	private:
		InviteCommand(const InviteCommand &other);
		InviteCommand &operator=(const InviteCommand &other);

	public:
		InviteCommand();
		~InviteCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif