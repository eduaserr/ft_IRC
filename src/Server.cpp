#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Replies.hpp"
#include "../include/commands/PingCommand.hpp"

void Server::_initSocket()
{
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd == -1)
		throw std::runtime_error("socket() failed");

	std::cout << "Fd : "<< _serverFd << std::endl;
	int reuse = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
	{
		close(_serverFd);
		_serverFd = -1;
		throw std::runtime_error("setsockopt() failed");
	}

	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(_serverFd);
		_serverFd = -1;
		throw std::runtime_error("fcntl() failed");
	}

	sockaddr_in address;
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(_port);

	if (bind(_serverFd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == -1)
	{
		close(_serverFd);
		_serverFd = -1;
		throw std::runtime_error("bind() failed");
	}

	if (listen(_serverFd, SOMAXCONN) == -1)
	{
		close(_serverFd);
		_serverFd = -1;
		throw std::runtime_error("listen() failed");
	}

	struct pollfd pfd;
	pfd.fd = _serverFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
	std::cout << "Server initialized on port " << _port << std::endl;
}

Server::Server(int port, const std::string& password) : _port(port), _password(password), _serverFd(-1), _registry()
{
	_initSocket();
	_registry.registerCommand("PING", new PingCommand());
}

Server::~Server()
{
	for (std::map<int, Client *>::iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	if (_serverFd != -1)
		close(_serverFd);
}

void Server::_acceptClient()
{
	int clientFd = accept(_serverFd, NULL, NULL);
	if (clientFd == -1)
		throw std::runtime_error("accept() failed");
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(clientFd);
		throw std::runtime_error("fcntl() client failed");
	}
	_clients[clientFd] = new Client(clientFd);
	struct pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
	std::cout << "[DEBUG] accepted client fd=" << clientFd << std::endl;
}

void Server::_handleClient(int fd)
{
	char buffer[512];
	ssize_t bytesRead;

	bytesRead = recv(fd, buffer, sizeof(buffer), 0);
	if (bytesRead == 0)
	{
		std::cout << "[DEBUG] client fd=" << fd << " disconnected" << std::endl;
		_removeClient(fd);
		return;
	}
	if (bytesRead < 0)
		throw std::runtime_error("recv() failed");

	std::string data(buffer, static_cast<std::size_t>(bytesRead));
	std::map<int, Client *>::iterator clientIt = _clients.find(fd);
	if (clientIt == _clients.end())
	{
		std::cout << "[DEBUG] client fd=" << fd
			<< " not found after recv" << std::endl;
		return;
	}
	Client *client = clientIt->second;

	std::cout << "[DEBUG] recv fd=" << fd << " bytes=" << bytesRead << " data=[" << data << "]" << std::endl;

	while (true)
	{
		Message message;
		Parser::Parse_e result = client->getParser().processInput(data, message);
		data.clear();

		std::cout << "[DEBUG] parser result=" << result << std::endl;
		if (result == Parser::PARSE_INCOMPLETE)
		{
			std::cout << "[DEBUG] incomplete input for fd=" << fd
				<< ", waiting for more bytes" << std::endl;
			break;
		}
		if (result == Parser::PARSE_INVALID)
		{
			std::cout << "[DEBUG] invalid IRC line for fd=" << fd
				<< ", message discarded" << std::endl;
			continue;
		}

		std::cout << "[DEBUG] command=[" << message.getCommand() << "]" << std::endl;
		if (!_registry.dispatch(*this, *client, message))
		{
			client->appendOutput(Replies::numeric("ircserv", "421",
				client->getNickname(), "Unknown command: "
				+ message.getCommand()));
			std::cout << "[DEBUG] unknown command for fd=" << fd << std::endl;
		}
		else
			std::cout << "[DEBUG] command dispatched for fd=" << fd << std::endl;
		for (std::size_t i = 0; i < _pollfds.size(); ++i)
		{
			if (_pollfds[i].fd == fd)
			{
				_pollfds[i].events |= POLLOUT;
				break;
			}
		}
	}
}

void Server::_flushClientOutput(int fd)
{
	Client *client = _clients[fd];
	std::string &output = client->getOutputBuffer();
	if (output.empty())
		return;
	ssize_t bytesSent = send(fd, output.data(), output.size(), 0);
	if (bytesSent < 0)
		throw std::runtime_error("send() failed");
	output.erase(0, static_cast<std::size_t>(bytesSent));
	std::cout << "[DEBUG] send fd=" << fd << " bytes=" << bytesSent << std::endl;
	if (output.empty())
	{
		for (std::size_t i = 0; i < _pollfds.size(); ++i)
		{
			if (_pollfds[i].fd == fd)
				_pollfds[i].events &= static_cast<short>(~POLLOUT);
		}
	}
}

void Server::_removeClient(int fd)
{
	std::map<int, Client *>::iterator it = _clients.find(fd);
	if (it != _clients.end())
	{
		delete it->second;
		_clients.erase(it);
	}
	for (std::vector<struct pollfd>::iterator pollIt = _pollfds.begin();
		pollIt != _pollfds.end(); ++pollIt)
	{
		if (pollIt->fd == fd)
		{
			_pollfds.erase(pollIt);
			break;
		}
	}
	close(fd);
}

void Server::run(){

	std::cout << "Server running. Waiting for connections..." << std::endl;
	while (g_running)
	{
		if (poll(_pollfds.data(), _pollfds.size(), -1) < 0){
			if (!g_running)
				break ;
			throw std::runtime_error("poll() failed");
		}
		for (size_t i = 0; i < _pollfds.size(); i++){
			int fd = _pollfds[i].fd;
			short revents = _pollfds[i].revents;
			if (fd == _serverFd){
				if (revents & POLLIN)
					_acceptClient();
				continue ;
			}
			if (revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				_removeClient(fd);
				continue ;
			}
			if (revents & POLLIN) { _handleClient(fd); }
			if (_clients.find(fd) == _clients.end()) { continue ; }
			if (revents & POLLOUT) { _flushClientOutput(fd); }
		}
	}
	std::cout << "Server shutting down." << std::endl;
}
