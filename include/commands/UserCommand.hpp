#ifndef USERCOMMAND_HPP
# define USERCOMMAND_HPP

#include "ICommand.hpp"

class UserCommand : public ICommand
{
	private:
		UserCommand(const UserCommand &other);
		UserCommand &operator=(const UserCommand &other);

	public:
		UserCommand();
		~UserCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif