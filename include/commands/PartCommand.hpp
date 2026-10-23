#ifndef PARTCOMMAND_HPP
# define PARTCOMMAND_HPP

#include "ICommand.hpp"

class PartCommand : public ICommand
{
	private:
		PartCommand(const PartCommand &other);
		PartCommand &operator=(const PartCommand &other);

	public:
		PartCommand();
		~PartCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif