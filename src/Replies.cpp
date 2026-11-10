#include "../include/Replies.hpp"

Replies::Replies()
{
}

std::string Replies::capLs(const std::string &serverName,
	const std::string &target, const std::string &capabilities)
{
	return ":" + serverName + " CAP " + target + " LS :"
		+ capabilities + "\r\n";
}

std::string Replies::capList(const std::string &serverName,
	const std::string &target, const std::string &capabilities)
{
	return ":" + serverName + " CAP " + target + " LIST :"
		+ capabilities + "\r\n";
}

std::string Replies::capAck(const std::string &serverName,
	const std::string &target, const std::string &capabilities)
{
	return ":" + serverName + " CAP " + target + " ACK :"
		+ capabilities + "\r\n";
}

std::string Replies::capNak(const std::string &serverName,
	const std::string &target, const std::string &capabilities)
{
	return ":" + serverName + " CAP " + target + " NAK :"
		+ capabilities + "\r\n";
}

std::string Replies::pong(const std::string &serverName, const std::string &token)
{
	return ":" + serverName + " PONG " + serverName + " :" + token + "\r\n";
}

std::string Replies::welcome(const std::string &serverName, const std::string &nickname)
{
	return numeric(serverName, "001", nickname, "Welcome to IRC server");
}

std::string Replies::nick(const std::string &prefix,
	const std::string &nickname)
{
	return ":" + prefix + " NICK " + nickname + "\r\n";
}

std::string Replies::join(const std::string &prefix, const std::string &channel)
{
	return ":" + prefix + " JOIN :" + channel + "\r\n";
}

std::string Replies::part(const std::string &prefix,
	const std::string &channel, const std::string &reason)
{
	std::string reply = ":" + prefix + " PART " + channel;

	if (!reason.empty())
		reply += " :" + reason;
	return reply + "\r\n";
}

std::string Replies::privmsg(const std::string &prefix,
	const std::string &target, const std::string &text)
{
	return ":" + prefix + " PRIVMSG " + target + " :"
		+ text + "\r\n";
}

std::string Replies::quit(const std::string &prefix,
	const std::string &reason)
{
	return ":" + prefix + " QUIT :" + reason + "\r\n";
}

std::string Replies::topic(const std::string &prefix,
	const std::string &channel, const std::string &text)
{
	return ":" + prefix + " TOPIC " + channel + " :"
		+ text + "\r\n";
}

std::string Replies::invite(const std::string &prefix,
	const std::string &nickname, const std::string &channel)
{
	return ":" + prefix + " INVITE " + nickname + " "
		+ channel + "\r\n";
}

std::string Replies::kick(const std::string &prefix,
	const std::string &channel, const std::string &nickname,
	const std::string &reason)
{
	std::string reply = ":" + prefix + " KICK " + channel
		+ " " + nickname;

	if (!reason.empty())
		reply += " :" + reason;
	return reply + "\r\n";
}

std::string Replies::mode(const std::string &prefix,
	const std::string &target, const std::string &modes,
	const std::string &parameters)
{
	std::string reply = ":" + prefix + " MODE " + target
		+ " " + modes;

	if (!parameters.empty())
		reply += " " + parameters;
	return reply + "\r\n";
}

std::string Replies::numeric(const std::string &serverName, const std::string &code, const std::string &target, const std::string &text)
{
	return ":" + serverName + " " + code + " " + target + " :" + text + "\r\n";
}

std::string Replies::topicReply(const std::string &serverName,
	const std::string &target, const std::string &channel,
	const std::string &topic)
{
	return ":" + serverName + " 332 " + target + " " + channel
		+ " :" + topic + "\r\n";
}

std::string Replies::noTopic(const std::string &serverName,
	const std::string &target, const std::string &channel)
{
	return ":" + serverName + " 331 " + target + " " + channel
		+ " :No topic is set\r\n";
}

std::string Replies::namesReply(const std::string &serverName,
	const std::string &target, const std::string &channel,
	const std::string &names)
{
	return ":" + serverName + " 353 " + target + " = " + channel
		+ " :" + names + "\r\n";
}

std::string Replies::endOfNames(const std::string &serverName,
	const std::string &target, const std::string &channel)
{
	return ":" + serverName + " 366 " + target + " " + channel
		+ " :End of /NAMES list\r\n";
}

std::string Replies::whoReply(const std::string &serverName,
	const std::string &target, const std::string &channel,
	const std::string &username, const std::string &host,
	const std::string &server, const std::string &nickname,
	bool operatorStatus, const std::string &realname)
{
	std::string status = "H";

	if (operatorStatus)
		status += "*";
	return ":" + serverName + " 352 " + target + " " + channel + " "
		+ username + " " + host + " " + server + " " + nickname + " "
		+ status + " :0 " + realname + "\r\n";
}

std::string Replies::whoEnd(const std::string &serverName,
	const std::string &target, const std::string &mask)
{
	return ":" + serverName + " 315 " + target + " " + mask
		+ " :End of /WHO list\r\n";
}

std::string Replies::whoisUser(const std::string &serverName,
	const std::string &target, const std::string &nickname,
	const std::string &username, const std::string &host,
	const std::string &realname)
{
	return ":" + serverName + " 311 " + target + " " + nickname + " "
		+ username + " " + host + " * :" + realname + "\r\n";
}

std::string Replies::whoisServer(const std::string &serverName,
	const std::string &target, const std::string &nickname,
	const std::string &reportedServer, const std::string &info)
{
	return ":" + serverName + " 312 " + target + " " + nickname + " "
		+ reportedServer + " :" + info + "\r\n";
}

std::string Replies::whoisEnd(const std::string &serverName,
	const std::string &target, const std::string &nickname)
{
	return ":" + serverName + " 318 " + target + " " + nickname
		+ " :End of /WHOIS list\r\n";
}

/*
1. CAP
2. PASS
3. NICK
4. USER
5. 001-004 de registro
6. JOIN
7. PART
8. PRIVMSG
9. QUIT
10. TOPIC
11. INVITE
12. KICK
13. MODE

WHO / WHOIS
*/