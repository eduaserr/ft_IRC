#ifndef JOINCOMMAND_HPP
# define JOINCOMMAND_HPP

#include "ICommand.hpp"

class JoinCommand : public ICommand
{
	private:
		JoinCommand(const JoinCommand &other);
		JoinCommand &operator=(const JoinCommand &other);

	public:
		JoinCommand();
		~JoinCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif