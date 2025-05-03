#ifndef CHANNELMANAGER_HPP
#define CHANNELMANAGER_HPP

#include <map>
#include <string>
#include <vector>

#include "Utils.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "IRCException.hpp"

class Client;
class ClientManager;
class Channel;
class Command;
class IRCServer;
class IRCException;

class ChannelManager {
   public:
	// Singleton
	static ChannelManager& getInstance();

	// Method for "channels"
	int countChannels() const;	
	void createChannel(const std::string& name, int creatorSocket);
	void removeChannel(const std::string name);
	Channel* getChannel(const std::string& name);
	void broadcastMessageAllChannels(const std::string& message, int senderSocket);

	// Method for Clean up
	void deleteAllChannels();
	void deleteClientsFromAllChannels(int clientSocket);

   private:
	// No instantiation outside for singletones!!
	ChannelManager() {}
	ChannelManager(const ChannelManager&);
	ChannelManager& operator=(const ChannelManager&);

	// Key : channel name / Value: Channel object
	std::map<std::string, Channel*> channels;
};

#endif	// CHANNELMANAGER_HPP
