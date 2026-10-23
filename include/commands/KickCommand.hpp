#ifndef KICKCOMMAND_HPP
# define KICKCOMMAND_HPP

#include "ICommand.hpp"

class KickCommand : public ICommand
{
	private:
		KickCommand(const KickCommand &other);
		KickCommand &operator=(const KickCommand &other);

	public:
		KickCommand();
		~KickCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif