#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include "../include/Replies.hpp"
#include "../include/commands/PingCommand.hpp"
#include "../include/commands/CapCommand.hpp"
#include "../include/commands/PassCommand.hpp"
#include "../include/commands/NickCommand.hpp"
#include "../include/commands/UserCommand.hpp"
#include "../include/commands/JoinCommand.hpp"
#include "../include/commands/InviteCommand.hpp"
#include "../include/commands/KickCommand.hpp"
#include "../include/commands/ModeCommand.hpp"
#include "../include/commands/TopicCommand.hpp"
#include "../include/commands/QuitCommand.hpp"
#include "../include/commands/PrivmsgCommand.hpp"
#include "../include/commands/WhoCommand.hpp"
#include "../include/commands/WhoisCommand.hpp"
#include "../include/commands/PartCommand.hpp"
#include <cerrno>

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

void Server::_registerCommands()
{
	_registry.registerCommand("PING", new PingCommand());
	_registry.registerCommand("CAP", new CapCommand());
	_registry.registerCommand("PASS", new PassCommand());
	_registry.registerCommand("NICK", new NickCommand());
	_registry.registerCommand("USER", new UserCommand());
	_registry.registerCommand("JOIN", new JoinCommand());
	_registry.registerCommand("PART", new PartCommand());
	_registry.registerCommand("PRIVMSG", new PrivmsgCommand());
	_registry.registerCommand("QUIT", new QuitCommand());
	_registry.registerCommand("TOPIC", new TopicCommand());
	_registry.registerCommand("INVITE", new InviteCommand());
	_registry.registerCommand("KICK", new KickCommand());
	_registry.registerCommand("MODE", new ModeCommand());
	_registry.registerCommand("WHO", new WhoCommand());
	_registry.registerCommand("WHOIS", new WhoisCommand());
}

Server::Server(int port, const std::string& password) : _port(port), _password(password), _serverFd(-1), _registry()
{
	_initSocket();
	_registerCommands();
}

Server::~Server()
{
	for (std::map<std::string, Channel *>::iterator it = _channels.begin();
		it != _channels.end(); ++it)
		delete it->second;
	_channels.clear();
	for (std::map<int, Client *>::iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	if (_serverFd != -1)
		close(_serverFd);
}

bool Server::nicknameInUse(const std::string &nickname, const Client *except) const
{
	for (std::map<int, Client *>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second != except && it->second->getNickname() == nickname)
			return true;
	}
	return false;
}

void Server::tryRegister(Client &client)
{
	if (!client.isRegistered() && client.hasAcceptedPassword() && !client.getNickname().empty() && !client.getUsername().empty())
	{
		client.setRegistered(true);
		client.appendOutput(Replies::welcome("ircserv", client.getNickname()));
		std::cout << "[AUTH] registered fd=" << client.getFd()
			<< " nick=" << client.getNickname() << std::endl;
	}
}

Channel *Server::getChannel(const std::string &name)
{
	std::map<std::string, Channel *>::iterator it = _channels.find(name);

	if (it == _channels.end())
		return NULL;
	return it->second;
}

Channel *Server::createChannel(const std::string &name)
{
	Channel *channel = getChannel(name);

	if (channel != NULL)
		return channel;
	channel = new Channel(name);
	_channels[name] = channel;
	return channel;
}

void Server::removeChannel(const std::string &name)
{
	std::map<std::string, Channel *>::iterator it = _channels.find(name);

	if (it == _channels.end())
		return ;
	delete it->second;
	_channels.erase(it);
}

Client *Server::getClient(const std::string &nickname)
{
	for (std::map<int, Client *>::iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return it->second;
	}
	return NULL;
}

void Server::disconnectClient(Client &client, const std::string &reason)
{
	std::vector<Client *> recipients;
	std::string message = Replies::quit(client.getPrefix(), reason);

	for (std::map<std::string, Channel *>::iterator channelIt = _channels.begin();
		channelIt != _channels.end(); ++channelIt)
	{
		if (!channelIt->second->isMember(&client))
			continue ;
		const std::vector<Client *> &members = channelIt->second->getMembers();
		for (std::size_t i = 0; i < members.size(); ++i)
		{
			if (members[i] == &client)
				continue ;
			if (std::find(recipients.begin(), recipients.end(), members[i])
				== recipients.end())
				recipients.push_back(members[i]);
		}
	}
	for (std::size_t i = 0; i < recipients.size(); ++i)
		sendToClient(recipients[i]->getFd(), message);
	_removeClient(client.getFd());
}

void Server::_acceptClient()
{
	sockaddr_in clientAddress;
	socklen_t clientAddressLength = sizeof(clientAddress);
	int clientFd = accept(_serverFd,
		reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressLength);
	if (clientFd == -1)
		return ;
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(clientFd);
		throw std::runtime_error("fcntl() client failed");
	}
	_clients[clientFd] = new Client(clientFd);
	_clients[clientFd]->setHost(inet_ntoa(clientAddress.sin_addr));
	struct pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
	std::cout << "[CONN] accepted fd=" << clientFd
		<< " host=" << _clients[clientFd]->getHost() << std::endl;
}

void Server::_handleClient(int fd)
{
	char buffer[512];
	ssize_t bytesRead;

	bytesRead = recv(fd, buffer, sizeof(buffer), 0);
	if (bytesRead == 0)
	{
		std::cout << "[CONN] closed fd=" << fd << std::endl;
		_removeClient(fd);
		return;
	}
	if (bytesRead < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ;
		std::cout << "[WARN] recv failed fd=" << fd << std::endl;
		_removeClient(fd);
		return ;
	}

	std::string data(buffer, static_cast<std::size_t>(bytesRead));
	std::map<int, Client *>::iterator clientIt = _clients.find(fd);
	if (clientIt == _clients.end())
	{
		return;
	}
	Client *client = clientIt->second;

	while (true)
	{
		Message message;
		Parser::Parse_e result = client->getParser().processInput(data, message);
		data.clear();

		if (result == Parser::PARSE_INCOMPLETE)
			break ;
		if (result == Parser::PARSE_INVALID)
		{
			std::cout << "[WARN] invalid IRC line fd=" << fd << std::endl;
			continue ;
		}

		std::cout << "[RX] fd=" << fd << " command="
			<< message.getCommand() << " params="
			<< message.getParams().size() << std::endl;
		if (!_registry.dispatch(*this, *client, message))
		{
			client->appendOutput(Replies::numeric("ircserv", "421", client->getNickname(), "Unknown command: " + message.getCommand()));
			std::cout << "[WARN] unknown command fd=" << fd << " command="
				<< message.getCommand() << std::endl;
		}
		if (_clients.find(fd) == _clients.end())
			break ;
		for (std::size_t i = 0; i < _pollfds.size(); ++i)
		{
			if (_pollfds[i].fd == fd)
			{
				_pollfds[i].events |= POLLOUT;
				break ;
			}
		}
	}
}

void Server::_flushClientOutput(int fd)
{
	Client *client = _clients[fd];
	std::string &output = client->getOutputBuffer();
	if (output.empty())
	{
		_setPollEvents(fd, POLLIN);
		return ;
	}
	ssize_t bytesSent = send(fd, output.data(), output.size(), 0);
	if (bytesSent < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ;
		_removeClient(fd);
		return ;
	}
	output.erase(0, static_cast<std::size_t>(bytesSent));
	std::cout << "[TX] fd=" << fd << " bytes=" << bytesSent << std::endl;
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
		Client *client = it->second;
		for (std::map<std::string, Channel *>::iterator channelIt = _channels.begin();
			channelIt != _channels.end();)
		{
			Channel *channel = channelIt->second;
			channel->removeMember(client);
			if (channel->isEmpty())
			{
				delete channel;
				_channels.erase(channelIt++);
			}
			else
				++channelIt;
		}
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


void	Server::_setPollEvents(int fd, short events)
{
	for (size_t i = 0; i < _pollfds.size(); i++)
		if (_pollfds[i].fd == fd) { _pollfds[i].events = events; break ; }
}

void	Server::sendToClient(int fd, const std::string &msg)
{
	std::map<int, Client *>::iterator	clientIt = _clients.find(fd);
	if (clientIt == _clients.end()) { return ; }

	clientIt->second->appendOutput(msg);
	_setPollEvents(fd, POLLIN | POLLOUT);
}

void	Server::broadcast(Channel &channel, const std::string &msg, Client *except)
{
	const std::vector<Client *> &members = channel.getMembers();

	for (std::size_t i = 0; i < members.size(); ++i)
	{
		if (members[i] != except)
			sendToClient(members[i]->getFd(), msg);
	}
}
