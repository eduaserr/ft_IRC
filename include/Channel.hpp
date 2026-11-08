/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eduaserr < eduaserr@student.42malaga.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 12:39:24 by erico-ke          #+#    #+#             */
/*   Updated: 2026/11/07 21:45:34 by eduaserr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <vector>
# include <algorithm>

class	Client;
class	Server;

class	Channel
{
	private:
		std::string				_name;
		std::string				_topic;
		std::string				_key;
		std::vector<Client *>	_members;
		std::vector<Client *>	_operators;
		std::vector<Client *>	_invited;
		bool					_inviteOnly;
		bool					_topicRestricted;
		int						_userLimit;

	public:
		Channel();
		Channel(const std::string &name);
		~Channel(void);

		// * Member methods * //
		void	addMember(Client *client);
		void	removeMember(Client *client);
		bool	isMember(Client *client)		const;
		bool	isEmpty()						const;
		// * Operator methods * //

		void	addOperator(Client *client);
		void	removeOperator(Client *client);
		bool	isOperator(Client *client) const;
		//* Invite methods * //

		void	addInvite(Client *client);
		bool	isInvited(Client *client) const;

		// * Broadcast (used to send message to each channel and user) * //
		void	broadcast(const std::string &msg, Server &server, Client *except = NULL);

		// * Getters * //
		std::string	getName()			const;
		std::string	getTopic()			const;
		std::string	getKey()			const;
		bool		isInviteOnly()		const;
		bool		isTopicRestricted()	const;
		int			getUserLimit()		const;
		size_t		getMemberCount()	const;

		// * Setters * //
		void	setTopic(const std::string &topic);
		void	setKey(const std::string &key);
		void	setInviteOnly(bool val);
		void	setTopicRestricted(bool val);
		void	setUserLimit(int limit);
		const	std::vector<Client*> &getMembers() const;
};

#endif