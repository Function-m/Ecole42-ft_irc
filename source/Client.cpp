#include "Client.hpp"

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sstream>
#include <stdexcept>

#include "ChannelManager.hpp"
#include "ClientManager.hpp"
#include "IRCException.hpp"

Client::Client(int socket) : clientSocket(socket), registrationStep(PASS) {
	// Set the client socket to non-blocking
	// int flags = fcntl(this->clientSocket, F_GETFL, 0);
	fcntl(this->clientSocket, F_SETFL, O_NONBLOCK);
}

Client::~Client() {
	close(this->clientSocket);
}

std::string Client::getUserInfo(int idx) const {
	if (idx < 0 || idx >= INFO_COUNT) {
		throw IRCException::ServerGeneralError("Invalid index for getUserInfo");
	}
	return this->userInfo[idx];
}

void Client::setUserInfo(int idx, const std::string& info) {
	if (idx < 0 || idx >= INFO_COUNT) {
		throw IRCException::ServerGeneralError("Invalid index for setUserInfo");
	}
	this->userInfo[idx] = info;
}

int Client::getRegistrationStep() const {
	return this->registrationStep;
}

void Client::setRegistrationStep(int step) {
	if (step < 0 || step >= INFO_COUNT) {
		throw IRCException::ServerGeneralError("Invalid registration step");
	}
	this->registrationStep = step;
}

void Client::sendMessage(std::string message) {
	while (message.size() > 512) {
		std::string part = message.substr(0, 512);
		message.erase(0, 512);
		if (send(this->clientSocket, part.c_str(), part.size(), 0) < 0) {
			throw IRCException::ServerGeneralError("send failed");
		}
		Utils::logDebug("Sent message to client " + Utils::toString(clientSocket) + "\n" + part.c_str() + "==END==");
	}
	if (message.size() > 0 && send(this->clientSocket, message.c_str(), message.size(), 0) < 0) {
		throw IRCException::ServerGeneralError("send failed");
	}
	Utils::logDebug("Sent message to client " + Utils::toString(clientSocket) + "\n" + message + "==END==");
}

std::vector<std::string> Client::receiveMessages() {
	char buffer[1024];
    int bytesReceived = recv(this->clientSocket, buffer, sizeof(buffer) - 1, 0);
    std::vector<std::string> messages;
	if (bytesReceived == 1023) {
		throw IRCException::ClientGeneralError("Message too long");
	}
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
		Utils::logDebug("Client " + Utils::toString(clientSocket) + " sent following message\n" + buffer + "==END==");
        this->commandBuffer += buffer;

        std::string::size_type pos;
        while ((pos = this->commandBuffer.find("\n")) != std::string::npos) {
            std::string message = this->commandBuffer.substr(0, pos);
            if (!message.empty() && *(message.rbegin()) == '\r') {
                message.erase(message.end() - 1);
            }
            messages.push_back(message);
            this->commandBuffer.erase(0, pos + 1);
        }
    } else if (bytesReceived == 0) {
        throw IRCException::ClientFatalError("Client connection closed");
    } else if (bytesReceived < 0) {
        throw IRCException::ClientFatalError("recv failed");
    }
    return messages;
}

std::string Client::generatePrefix(std::string permission) {
	return ":" + permission + this->getUserInfo(Client::NICKNAME) + "!" + this->getUserInfo(Client::USER_NAME) + "@" + this->getUserInfo(Client::SERVER_NAME);
}

int Client::getClientSocket() const {
	return clientSocket;
}
