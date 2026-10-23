#ifndef PASSCOMMAND_HPP
# define PASSCOMMAND_HPP

#include "ICommand.hpp"

class PassCommand : public ICommand
{
	private:
		PassCommand(const PassCommand &other);
		PassCommand &operator=(const PassCommand &other);

	public:
		PassCommand();
		~PassCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif