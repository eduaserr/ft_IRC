#include "inc/Server.hpp"

//parse libs
#include <string>
#include <iostream>
#include <sstream>
#include <cctype>
#include <cstdlib>

#include <csignal>

volatile sig_atomic_t g_running = true;

void	signalHandler(int sig)
{
	(void)sig;
	g_running = false;
}

using namespace std;

bool parsePort(const string& s, int &port){
	if (s.empty() || s.size() > 5)
		return false;
	for (size_t i = 0; i < s.size(); i++){
		if (!isdigit(static_cast<unsigned char>(s[i])))
			return false;
	}
	stringstream ss(s);

	if (!(ss >> port) || !ss.eof() || port < 1 || port > 65535)
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

int main(int ac, char **av){

	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGPIPE, signalHandler);

	int port = 0;

	if (ac != 3)
		return (cout << "ERR_N1" << endl, 1);
	if (!parsePort(av[1], port) || !parsePasswd(av[2]))
		return (cout << "ERR_N2" << endl, 1);

	cout << port << ", "<< av[2] << endl;
	try
	{
		Server server(port, av[2]);
	}
	catch (const std::runtime_error &error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
		return 1;
	}
	return 0;
}