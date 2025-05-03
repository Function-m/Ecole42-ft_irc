#include "IRCServer.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

#include <sstream>
#include <stdexcept>

#include "ChannelManager.hpp"
#include "ClientManager.hpp"
#include "Command.hpp"
#include "Utils.hpp"
#include "IRCException.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Utils.hpp"
#include "Utils.hpp"

IRCServer &IRCServer::getInstance() {
	static IRCServer instance;
	return instance;
}

IRCServer::~IRCServer() {
	for (std::vector<struct pollfd>::iterator it = this->pollfds.begin(); it != this->pollfds.end(); ++it) {
		close(it->fd);
	}
	ClientManager::getInstance().deleteAllClients();
	ChannelManager::getInstance().deleteAllChannels();
}

void IRCServer::run(int port, const std::string &password) {
	// Initialize the server
	this->serverPassword = password;
	this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->serverSocket < 0) {
		throw IRCException::ServerFatalError("Socket creation failed");
	}
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(this->serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		throw IRCException::ServerFatalError("Socket binding failed");
	}
	if (listen(this->serverSocket, 5) < 0) {
		throw IRCException::ServerFatalError("Socket listening failed");
	}
	struct pollfd pfd;
	pfd.fd = this->serverSocket;
	pfd.events = POLLIN;
	this->pollfds.push_back(pfd);

	// Run the server
	while (true) {
		try {
			if (poll(&this->pollfds[0], this->pollfds.size(), -1) < 0) {
				throw IRCException::ServerGeneralError("Poll failed");
			}
			for (size_t i = 0; i < this->pollfds.size(); ++i) {
				// Accept new client
				if (!(this->pollfds[i].revents & POLLIN)) {
					continue;
				}
				if (this->pollfds[i].fd == this->serverSocket) {
					int clientSocket = accept(this->serverSocket, NULL, NULL);
					if (clientSocket < 0) {
						throw IRCException::ServerGeneralError("Client connection acceptance failed");
					}
					ClientManager::getInstance().addClient(clientSocket);
					this->addPollfds(clientSocket);
					Utils::logInfo("New client connected with socket: " + Utils::toString(clientSocket));

				// Process client message
				} else {
					this->processClientMessage(this->pollfds[i].fd);
				}
			}
		} catch (const IRCException::ServerGeneralError &e) {
			Utils::logWarning("General exception occurred while running the server: " + std::string(e.what()));
		}
	}
}

std::string IRCServer::getServerPassword() const {
	return serverPassword;
}

void IRCServer::setServerPassword(const std::string& password) {
	serverPassword = password;
}

void IRCServer::processClientMessage(int clientSocket) {
	Client *client = ClientManager::getInstance().getClient(clientSocket);

	try {
		std::vector<std::string> messages = client->receiveMessages();
		for (std::vector<std::string>::iterator it = messages.begin(); it != messages.end(); ++it) {
			std::istringstream iss(*it);
			std::string commandName;
			std::vector<std::string> args;
			std::string arg;

			iss >> commandName;
			while (iss >> arg) {
				args.push_back(arg);
			}

			if (client->getRegistrationStep() != Client::REGISTERED) {
				Command::getInstance().handleRegistration(clientSocket, commandName, args);
			} else {
				Command::getInstance().handleCommand(clientSocket, commandName, args);
			}
		}
	} catch (const IRCException::ClientGeneralError &e) {
		Utils::logWarning("General exception occurred while processing client message: " + std::string(e.what()));
		ClientManager::getInstance().sendMessageToClient(clientSocket, std::string(e.what()) + "\r\n");
	} catch (const IRCException::ClientFatalError &e) {
		Utils::logWarning("Fatal exception occurred while processing client message: " + std::string(e.what()));
		ChannelManager::getInstance().deleteClientsFromAllChannels(clientSocket);
		ClientManager::getInstance().deleteClient(clientSocket);
	}
}

void IRCServer::addPollfds(int clientSocket) {
	struct pollfd pfd;
	pfd.fd = clientSocket;
	pfd.events = POLLIN;
	this->pollfds.push_back(pfd);
}

void IRCServer::removePollfds(int clientSocket) {
	for (std::vector<struct pollfd>::iterator it = this->pollfds.begin(); it != this->pollfds.end(); ++it) {
		if (it->fd == clientSocket) {
			this->pollfds.erase(it);
			break;
		}
	}
}
