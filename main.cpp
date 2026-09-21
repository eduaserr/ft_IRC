#include "inc/file.hpp"

//parse libs
#include <string>
#include <sstream>
#include <cctype>

#include <csignal>

bool	g_running = true;

void	signalHandler(int sig)
{
	(void)sig;
	g_running = false;
}

using namespace std;

bool parsePort(const string& s){
	if (s.empty() || s.size() > 5)
		return false;
	for (size_t i = 0; i < s.size(); i++){
		if (!isdigit(s[i]))
			return false;
	}
	stringstream ss(s);
	int port;

	if (!(ss >> port) || !ss.eof() || port > 65535)
		return false;
	return true;
}
bool parsePasswd(const string& s){
	if (s.empty())
		return false;
	for (size_t i = 0; i < s.size(); i++){
		unsigned char c = s[i];
		if (!isprint(c))
			return false;
	}
	return true;
}
bool parseInput(char** av){

	if (!parsePort(av[1]) || !parsePasswd(av[2]))
		return false;
	return true;
}
int main(int ac, char **av){

	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGPIPE, signalHandler);

	if (ac != 3)
		return (cout << "ERR_N1" << endl, 1);
	if (!parseInput(av))
		return (cout << "ERR_N2" << endl, 1);
	return 0;
}