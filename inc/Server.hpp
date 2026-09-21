#ifndef SERVER_HPP
# define SERVER_HPP

#include <csignal>
# include <vector>
# include <map>
# include <string>
# include <poll.h>
# include <stdexcept>
# include <iostream>
# include <cstring>
# include <unistd.h>
# include <fcntl.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <netinet/in.h>

extern volatile sig_atomic_t	g_running;

class	Server
{
	private:
		Server();
		Server(const Server &other);
		Server &operator=(const Server &other);

		int				_port;
		std::string		_password;
		int				_serverFd;
		std::vector<struct pollfd> _pollfds;

		void _initSocket();

	public:
		Server(int port, const std::string &password);
		~Server();

		//void	run();

		// * Getters * //
		const std::string&	getPassword() const {return _password;}
};

#endif