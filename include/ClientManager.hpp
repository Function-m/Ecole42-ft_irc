#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP

#include <map>
#include <string>

#include "Utils.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "IRCException.hpp"

class Client;
class Channel;
class ChannelManager;
class Command;
class IRCServer;
class IRCException;

class ClientManager {
   public:
	// Singleton
	static ClientManager& getInstance();

	// For communication
	void sendMessageToClient(int clientSocket, const std::string& message);

	// Methods for "clients"

	// Methods for "clients"
	void addClient(int clientSocket);
	int countClients() const;
	Client* getClient(int clientSocket);
	Client* getClient(const std::string& nickname);
	int getClientSocket(const std::string& nickname) const;
	int getClientSocket(const Client* client) const;

	// Method for Clean up
	void deleteAllClients();
	void deleteClient(int clientSocket);

   private:
	// No instantiation outside for singletones!!
	ClientManager() {}
	ClientManager(const ClientManager&);
	ClientManager& operator=(const ClientManager&);

	// Key : client socket / Value: Client object
	std::map<int, Client*> clients;
};

#endif	// CLIENTMANAGER_HPP
