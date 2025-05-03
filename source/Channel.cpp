#include "Channel.hpp"

#include <algorithm>

#include "ChannelManager.hpp"
#include "ClientManager.hpp"
#include "IRCException.hpp"

Channel::Channel(const std::string& name, int creatorSocket) {
	this->channelInfo[NAME] = name;
	this->channelInfo[TOPIC] = "";
	this->channelInfo[LIMIT] = "0";
	this->channelInfo[PASSWORD] = "";
	this->channelInfo[INVITE_ONLY] = "0";

	addClientSocket(creatorSocket);
	addOperatorSocket(creatorSocket);
}

Channel::~Channel() {
	// TODO: Implement
}

bool Channel::hasClientSocket(int clientSocket) const {
	return this->clientSockets.find(clientSocket) != this->clientSockets.end();
}

void Channel::addClientSocket(int clientSocket) {
	this->clientSockets.insert(clientSocket);
}

void Channel::removeClientSocket(int clientSocket) {
	this->clientSockets.erase(clientSocket);
	this->removeOperatorSocket(clientSocket);
	if (this->clientSockets.empty()) {
		ChannelManager::getInstance().removeChannel(this->channelInfo[NAME]);
	}
}

int Channel::countClientSockets() const {
	return this->clientSockets.size();
}

bool Channel::isInviteSocket(int clientSocket) const {
	if (this->inviteSockets.empty()) {
		return false;
	}
	return this->inviteSockets.find(clientSocket) != this->inviteSockets.end();
}

void Channel::addInviteSocket(int clientSocket) {
	this->inviteSockets.insert(clientSocket);
}

void Channel::removeInviteSocket(int clientSocket) {
	this->inviteSockets.erase(clientSocket);
}

void Channel::setInfo(int idx, const std::string info) {
	if (info == this->channelInfo[idx]) {
		return;
	}
	this->channelInfo[idx] = info;
}

std::string Channel::getInfo(int idx) const {
	if (idx < 0 || idx >= INFO_COUNT) {
		throw IRCException::ServerGeneralError("Invalid index for getInfo");
	}
	return this->channelInfo[idx];
}

void Channel::broadcastMessage(const std::string& message, int senderSocket) {
	ClientManager& clientManager = ClientManager::getInstance();
	for (std::set<int>::iterator it = this->clientSockets.begin(); it != this->clientSockets.end(); ++it) {
		if (*it != senderSocket) {
			(clientManager.getClient(*it))->sendMessage(message);
		}
	}
}

bool Channel::isOperatorSocket(int clientSocket) const {
	return this->operatorSockets.find(clientSocket) != this->operatorSockets.end();
}

void Channel::addOperatorSocket(int clientSocket) {
	this->operatorSockets.insert(clientSocket);
}

void Channel::removeOperatorSocket(int clientSocket) {
	this->operatorSockets.erase(clientSocket);
	if (!this->clientSockets.empty() && this->operatorSockets.empty()) {
		this->addOperatorSocket(*this->clientSockets.begin());
	}
}

// Add this function
const std::set<int>& Channel::getClientSockets() const {
	return this->clientSockets;
}
