#ifndef QUITCOMMAND_HPP
# define QUITCOMMAND_HPP

#include "ICommand.hpp"

class QuitCommand : public ICommand
{
	private:
		QuitCommand(const QuitCommand &other);
		QuitCommand &operator=(const QuitCommand &other);

	public:
		QuitCommand();
		~QuitCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif