#include "ClientManager.hpp"

#include "ChannelManager.hpp"
#include "Client.hpp"
#include "Utils.hpp"

ClientManager& ClientManager::getInstance() {
	static ClientManager instance;
	return instance;
}

void ClientManager::addClient(int clientSocket) {
	Client* client = new Client(clientSocket);
	this->clients[clientSocket] = client;
}

void ClientManager::deleteAllClients() {
	for (std::map<int, Client*>::iterator it = this->clients.begin(); it != this->clients.end(); ++it) {
		delete it->second;
	}
	this->clients.clear();
}

void ClientManager::deleteClient(int clientSocket) {
	if (this->clients.find(clientSocket) != this->clients.end()) {
		delete this->clients[clientSocket];
		this->clients.erase(clientSocket);
		ChannelManager::getInstance().deleteClientsFromAllChannels(clientSocket);
	}
}

Client* ClientManager::getClient(int clientSocket) {
	if (this->clients.find(clientSocket) != this->clients.end()) {
		return this->clients[clientSocket];
	} else {
		return NULL;
	}
}

Client* ClientManager::getClient(const std::string& nickname) {
	for (std::map<int, Client*>::iterator it = this->clients.begin(); it != this->clients.end(); ++it) {
		if (it->second->getUserInfo(Client::NICKNAME) == nickname) {
			return it->second;
		}
	}
	return NULL;
}

int ClientManager::getClientSocket(const std::string& nickname) const {
	for (std::map<int, Client*>::const_iterator it = this->clients.begin(); it != clients.end(); ++it) {
		if (it->second->getUserInfo(Client::NICKNAME) == nickname) {
			return it->first;
		}
	}
	return -1;
}

int ClientManager::getClientSocket(const Client* client) const {
	for (std::map<int, Client*>::const_iterator it = this->clients.begin(); it != clients.end(); ++it) {
		if (it->second == client) {
			return it->first;
		}
	}
	return -1;
}

int ClientManager::countClients() const {
	return this->clients.size();
}

void ClientManager::sendMessageToClient(int clientSocket, const std::string& message) {
	Client* client = this->getClient(clientSocket);
	if (client) {
		client->sendMessage(message);
	}
}
