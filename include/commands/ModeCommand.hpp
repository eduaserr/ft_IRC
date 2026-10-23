#ifndef MODECOMMAND_HPP
# define MODECOMMAND_HPP

#include "ICommand.hpp"

class ModeCommand : public ICommand
{
	private:
		ModeCommand(const ModeCommand &other);
		ModeCommand &operator=(const ModeCommand &other);

	public:
		ModeCommand();
		~ModeCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif