#ifndef PRIVMSGCOMMAND_HPP
# define PRIVMSGCOMMAND_HPP

#include "ICommand.hpp"

class PrivmsgCommand : public ICommand
{
	private:
		PrivmsgCommand(const PrivmsgCommand &other);
		PrivmsgCommand &operator=(const PrivmsgCommand &other);

	public:
		PrivmsgCommand();
		~PrivmsgCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif