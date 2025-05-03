#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <set>
#include <poll.h>

#include "Utils.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "IRCException.hpp"

class ClientManager;
class Channel;
class ChannelManager;
class Command;
class IRCServer;
class IRCException;

class Client {
   public:
	Client(int socket);
	~Client();

	// For communication
	void sendMessage(std::string message);
	std::vector<std::string> receiveMessages();
	std::string generatePrefix(std::string permission = "");

	// Methods for "clientSocket"
	int getClientSocket() const;

	// Methods for "registrationStep"
	enum e_registrationStep { PASS, NICK, USER, REGISTERED, REGISTRATION_COUNT };
	int getRegistrationStep() const;
	void setRegistrationStep(int step);

	// Methods for "UserInfo"
	enum e_userInfo { NICKNAME, USER_NAME, HOST_NAME, SERVER_NAME, REAL_NAME, INFO_COUNT };
	std::string getUserInfo(int idx) const;
	void setUserInfo(int idx, const std::string& info);
	
   private:
	int clientSocket;
	int registrationStep;
	std::string userInfo[INFO_COUNT];
	std::string commandBuffer;
};

#endif	// CLIENT_HPP
