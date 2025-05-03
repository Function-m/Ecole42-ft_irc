#include "ChannelManager.hpp"
#include "ClientManager.hpp"
#include "Client.hpp"

ChannelManager& ChannelManager::getInstance() {
    static ChannelManager instance;
    return instance;
}

Channel* ChannelManager::getChannel(const std::string& name) {
    if (this->channels.find(name) != this->channels.end()) {
        return this->channels[name];
    } else {
        return NULL;
    }
}

void ChannelManager::createChannel(const std::string& name, int creatorSocket) {
    if (this->channels.find(name) == this->channels.end()) {
        this->channels[name] = new Channel(name, creatorSocket);
    }
}

void ChannelManager::removeChannel(const std::string name) {
    if (this->channels.find(name) != this->channels.end()) {
        delete this->channels[name];
        this->channels.erase(name);
    }
}

int ChannelManager::countChannels() const {
    return this->channels.size();
}

void ChannelManager::deleteAllChannels() {
	for (std::map<std::string, Channel*>::iterator it = this->channels.begin(); this->channels.size() != 0 && it != this->channels.end(); ++it) {
        delete it->second;
    }
    this->channels.clear();
}

void ChannelManager::deleteClientsFromAllChannels(int clientSocket) {
    std::set<std::string> channelsToDelete;

    for (std::map<std::string, Channel*>::iterator it = this->channels.begin(); it != this->channels.end(); ++it) {
        if (it->second->isInviteSocket(clientSocket)) {
            it->second->removeInviteSocket(clientSocket);
        }
        if (it->second->hasClientSocket(clientSocket)) {
            channelsToDelete.insert(it->first);
        }
    }

    for (std::set<std::string>::iterator it = channelsToDelete.begin(); it != channelsToDelete.end(); ++it) {
        removeChannel(*it);
    }
}

void ChannelManager::broadcastMessageAllChannels(const std::string& message, int senderSocket) {
	for (std::map<std::string, Channel*>::iterator it = this->channels.begin(); it != this->channels.end(); ++it) {
		if (senderSocket == -1) {
			it->second->broadcastMessage(message, senderSocket);
		} else if (it->second->hasClientSocket(senderSocket)) {
			it->second->broadcastMessage(message, senderSocket);
		}
	}
}
