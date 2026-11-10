#ifndef WHOISCOMMAND_HPP
# define WHOISCOMMAND_HPP

#include "ICommand.hpp"

class WhoisCommand : public ICommand
{
	private:
		WhoisCommand(const WhoisCommand &other);
		WhoisCommand &operator=(const WhoisCommand &other);

	public:
		WhoisCommand();
		~WhoisCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif
