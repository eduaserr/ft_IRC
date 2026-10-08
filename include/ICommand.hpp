#ifndef ICOMMAND_HPP
# define ICOMMAND_HPP

class Server;
class Client;
class Message;

class ICommand
{
	private:
		ICommand(const ICommand &other);
		ICommand &operator=(const ICommand &other);

	protected:
		ICommand();

	public:
		virtual ~ICommand();

		virtual void execute(Server &server, Client &client,
			const Message &message) = 0;
};

#endif