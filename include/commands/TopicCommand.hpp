#ifndef TOPICCOMMAND_HPP
# define TOPICCOMMAND_HPP

#include "ICommand.hpp"

class TopicCommand : public ICommand
{
	private:
		TopicCommand(const TopicCommand &other);
		TopicCommand &operator=(const TopicCommand &other);

	public:
		TopicCommand();
		~TopicCommand();

		void execute(Server &server, Client &client, const Message &message);
};

#endif