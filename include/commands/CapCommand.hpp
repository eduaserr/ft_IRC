#ifndef CAPCOMMAND_HPP
# define CAPCOMMAND_HPP

#include "../../include/Client.hpp"
#include "../../include/Message.hpp"
#include "../../include/Replies.hpp"
#include <string>
#include <vector>

#include "ICommand.hpp"

class CapCommand : public ICommand
{
	private:
		CapCommand(const CapCommand &other);
		CapCommand &operator=(const CapCommand &other);

	public:
		CapCommand();
		~CapCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif