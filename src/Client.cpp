#include "../include/Client.hpp"

Client::Client(int fd) : _fd(fd), _host(), _nickname(), _username(),
	_realname(), _passwordAccepted(false), _registered(false), _inputBuffer(),
	_outputBuffer()
{
}

Client::~Client()
{
}

int Client::getFd() const
{
	return _fd;
}

const std::string& Client::getHost() const
{
	return _host;
}

const std::string& Client::getNickname() const
{
	return _nickname;
}

const std::string& Client::getUsername() const
{
	return _username;
}

const std::string& Client::getRealname() const
{
	return _realname;
}

bool Client::hasAcceptedPassword() const
{
	return _passwordAccepted;
}

bool Client::isRegistered() const
{
	return _registered;
}

void Client::setHost(const std::string &host)
{
	_host = host;
}

void Client::setNickname(const std::string &nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string &username)
{
	_username = username;
}

void Client::setRealname(const std::string &realname)
{
	_realname = realname;
}

void Client::setPasswordAccepted(bool accepted)
{
	_passwordAccepted = accepted;
}

void Client::setRegistered(bool registered)
{
	_registered = registered;
}

std::string& Client::getInputBuffer()
{
	return _inputBuffer;
}

std::string& Client::getOutputBuffer()
{
	return _outputBuffer;
}

void Client::appendOutput(const std::string &message)
{
	_outputBuffer += message;
}

Parser& Client::getParser()
{
	return _parser;
}
