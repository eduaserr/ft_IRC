#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>

class Client
{
private:
	Client();
	Client(const Client &other);
	Client &operator=(const Client &other);

	int _fd;
	std::string _host;

	std::string _nickname;
	std::string _username;
	std::string _realname;

	bool _passwordAccepted;
	bool _registered;

	std::string _inputBuffer;
	std::string _outputBuffer;

public:
	Client(int fd);
	~Client();

	int getFd() const;
	const std::string& getHost() const;
	const std::string& getNickname() const;
	const std::string& getUsername() const;
	const std::string& getRealname() const;

	bool hasAcceptedPassword() const;
	bool isRegistered() const;

	void setHost(const std::string &host);
	void setNickname(const std::string &nickname);
	void setUsername(const std::string &username);
	void setRealname(const std::string &realname);
	void setPasswordAccepted(bool accepted);
	void setRegistered(bool registered);

	std::string& getInputBuffer();
	std::string& getOutputBuffer();
	void appendOutput(const std::string &message);
};

#endif