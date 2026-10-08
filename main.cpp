#include "Config.hpp"
#include "Server.hpp"
#include <iostream>
#include <exception>


#include "Parser.hpp"
int	main(int argc, char** argv)
{
	//(void)argc;
	//(void)argv;
	Config::installSignalHandlers();
	try
	{
		Config	config(argc, argv);
		Server	server(config.getPort(), config.getPasswd());

		//Message msg;
		//Parser obj;

		//std::string data(":nick!user@host PRIVMSG #general :Hola\r\n");

		//std::cout << msg.getPrefix() << std::endl;
		//std::cout << msg.getCommand() << std::endl;


		//obj.processInput(data, msg);

		//std::cout << msg.getPrefix() << std::endl;
		//std::cout << msg.getCommand() << std::endl;
		server.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "ircserv: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}


//:nick!user@host PRIVMSG #general :Hola a todos