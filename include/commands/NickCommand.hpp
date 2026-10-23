#ifndef NICKCOMMAND_HPP
# define NICKCOMMAND_HPP

#include "ICommand.hpp"

class NickCommand : public ICommand
{
	private:
		NickCommand(const NickCommand &other);
		NickCommand &operator=(const NickCommand &other);

	public:
		NickCommand();
		~NickCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif