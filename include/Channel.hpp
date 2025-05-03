#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>

#include "Utils.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "IRCException.hpp"

class Client;
class ClientManager;
class ChannelManager;
class Command;
class IRCServer;
class IRCException;

class Channel {
   public:
	Channel(const std::string& name, int creatorSocket);
	~Channel();

	// For communication
	void broadcastMessage(const std::string& message, int senderSocket);

	// Methods for "channelInfo"
	enum e_chatInfo { NAME, TOPIC, TOPIC_PERMISSION, LIMIT, PASSWORD, INVITE_ONLY, INFO_COUNT };
	void setInfo(int idx, const std::string info = "");
	std::string getInfo(int idx) const;

	// Methods for "clientSockets"
	bool hasClientSocket(int clientSocket) const;
	void addClientSocket(int clientSocket);
	void removeClientSocket(int clientSocket);
	int countClientSockets() const;
	const std::set<int>& getClientSockets() const;  // Add this line

	// Methods for "inviteSockets"
	bool isInviteSocket(int clientSocket) const;
	void addInviteSocket(int clientSocket);
	void removeInviteSocket(int clientSocket);

	// Methods for "operatorSockets"
	bool isOperatorSocket(int clientSocket) const;
	void addOperatorSocket(int clientSocket);
	void removeOperatorSocket(int clientSocket);

   private:
	std::string channelInfo[INFO_COUNT];
	std::set<int> clientSockets;
	std::set<int> inviteSockets;
	std::set<int> operatorSockets;
};

#endif	// CHANNEL_HPP
