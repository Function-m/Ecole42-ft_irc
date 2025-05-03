#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <poll.h>

#include <iostream>
#include <string>
#include <vector>

#include "Utils.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "IRCException.hpp"

#define IRC_SERVER_NAME ":IRCServer"

class Client;
class ClientManager;
class Channel;
class ChannelManager;
class Command;
class IRCException;

class IRCServer {
   public:
	// Singleton
	static IRCServer& getInstance();
	~IRCServer();

	// Method for running a server
	void run(int port, const std::string& password);

	// Methods for "serverPassword"
	std::string getServerPassword() const;
	void setServerPassword(const std::string& password);

	// Methods for "pollfds"
	void addPollfds(int clientSocket);
	void removePollfds(int clientSocket);

   private:
	// No instantiation outside for singletones!!
	IRCServer() {}
	IRCServer(const IRCServer&);
	IRCServer& operator=(const IRCServer&);

	void processClientMessage(int clientSocket);

	int serverSocket;
	std::string serverPassword;
	std::vector<struct pollfd> pollfds;
};

#endif	// IRCSERVER_HPP
