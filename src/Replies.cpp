#include "../include/Replies.hpp"

Replies::Replies()
{
}

std::string Replies::pong(const std::string &serverName,
	const std::string &token)
{
	return ":" + serverName + " PONG " + serverName + " :" + token + "\r\n";
}

std::string Replies::numeric(const std::string &serverName, const std::string &code, const std::string &target, const std::string &text)
{
	return ":" + serverName + " " + code + " " + target + " :" + text + "\r\n";
}
