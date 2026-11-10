#ifndef WHOCOMMAND_HPP
# define WHOCOMMAND_HPP

#include "ICommand.hpp"

class WhoCommand : public ICommand
{
	private:
		WhoCommand(const WhoCommand &other);
		WhoCommand &operator=(const WhoCommand &other);

	public:
		WhoCommand();
		~WhoCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif
