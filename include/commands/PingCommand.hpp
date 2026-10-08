#ifndef PINGCOMMAND_HPP
# define PINGCOMMAND_HPP

#include "ICommand.hpp"

class PingCommand : public ICommand
{
	private:
		PingCommand(const PingCommand &other);
		PingCommand &operator=(const PingCommand &other);

	public:
		PingCommand();
		~PingCommand();

		void execute(Server &server, Client &client,
			const Message &message);
};

#endif
