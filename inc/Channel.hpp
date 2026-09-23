#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <set>
#include <cstddef>

class Client;

class Channel
{
private:
	Channel();
	Channel(const Channel &other);
	Channel &operator=(const Channel &other);

	std::string _name;
	std::string _topic;

	/*#include <set>

	Almacenamos el int Fd.

	std::set<int> _members;
	std::set<int> _operators;
	std::set<int> _invited;*/

	//desventaja: si server elimina un cliente, hay que quitar el puntero de todos los canales
	std::set<Client *> _members;
	std::set<Client *> _operators;
	std::set<Client *> _invited;

	bool _inviteOnly;
	bool _topicOperatorsOnly;

	bool _hasKey;
	std::string _key;

	bool _hasUserLimit;
	std::size_t _userLimit;

public:
	Channel(const std::string &name);
	~Channel();

	const std::string& getName() const;
	const std::string& getTopic() const;
	bool hasMember(Client *client) const;
	bool isOperator(Client *client) const;
	bool isInvited(Client *client) const;

	void addMember(Client *client);
	void removeMember(Client *client);
	void addOperator(Client *client);
	void removeOperator(Client *client);
	void invite(Client *client);
	void removeInvite(Client *client);
};

#endif