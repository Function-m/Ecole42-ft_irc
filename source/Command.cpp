#include "Command.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <sstream>
#include <stdexcept>
#include <iostream>
#include <ctime>
#include "ChannelManager.hpp"
#include "Client.hpp"
#include "ClientManager.hpp"
#include "IRCServer.hpp"
#include "Utils.hpp"
#include "IRCException.hpp"
#include "Channel.hpp"
#include "Utils.hpp"

#define IRC_SERVER_NAME ":IRCServer"

Command& Command::getInstance() {
    static Command instance;
    return instance;
}

Command::~Command() {
}

void Command::handleCommand(int clientSocket, const std::string& commandName, const std::vector<std::string>& args) {
    CommandHandler command = this->getCommand(commandName);
    if (command) {
        (this->*command)(clientSocket, args);
    } else {
        throw IRCException::ClientGeneralError("Unknown command");
    }
}

void Command::handleRegistration(int clientSocket, const std::string& commandName, const std::vector<std::string>& args) {
    Client* client = ClientManager::getInstance().getClient(clientSocket);

    CommandHandler command = this->getCommand(commandName);
    if (commandName == "CAP") {
        (this->*command)(clientSocket, args);
        Utils::logDebug("Client " + Utils::toString(clientSocket) + " used " + commandName + " command");

    } else if (commandName == "PASS" && client->getRegistrationStep() == Client::PASS) {
        (this->*command)(clientSocket, args);
        client->setRegistrationStep(Client::NICK);
        Utils::logDebug("Client " + Utils::toString(clientSocket) + " used " + commandName + " command");

    } else if (commandName == "NICK" && client->getRegistrationStep() == Client::NICK) {
        (this->*command)(clientSocket, args);
        client->setRegistrationStep(Client::USER);
        Utils::logDebug("Client " + Utils::toString(clientSocket) + " used " + commandName + " command");

    } else if (commandName == "USER" && client->getRegistrationStep() == Client::USER) {
        (this->*command)(clientSocket, args);
        client->setRegistrationStep(Client::REGISTERED);
        // Essential initial messages
        client->sendMessage(IRC_SERVER_NAME " 001 " + client->getUserInfo(Client::NICKNAME) + " :Welcome to the Internet Relay Network " + client->getUserInfo(Client::NICKNAME) + "\r\n"
        + IRC_SERVER_NAME " 002 " + client->getUserInfo(Client::NICKNAME) + " :Your host is IRCServer, running version 1.0\r\n"
        + IRC_SERVER_NAME " 003 " + client->getUserInfo(Client::NICKNAME) + " :This server was created Mon Jan 01 2020 at 00:00:00 GMT\r\n"
        + IRC_SERVER_NAME " 004 " + client->getUserInfo(Client::NICKNAME) + " IRCServer 1.0  itkol\r\n"
        + IRC_SERVER_NAME " 005 " + client->getUserInfo(Client::NICKNAME) + " CHANTYPES=# CHANMODES=itkol :are supported on this server\r\n"
        + IRC_SERVER_NAME " 251 " + client->getUserInfo(Client::NICKNAME) + " :There are " + Utils::toString(ClientManager::getInstance().countClients()) + " users and 0 services on 1 servers\r\n"
        + IRC_SERVER_NAME " 252 " + client->getUserInfo(Client::NICKNAME) + " 0 :operator(s) online\r\n"
        + IRC_SERVER_NAME " 253 " + client->getUserInfo(Client::NICKNAME) + " 0 :unknown connection(s)\r\n"
        + IRC_SERVER_NAME " 254 " + client->getUserInfo(Client::NICKNAME) + " " + Utils::toString(ChannelManager::getInstance().countChannels()) + " :channels formed\r\n"
        + IRC_SERVER_NAME " 255 " + client->getUserInfo(Client::NICKNAME) + " :I have " + Utils::toString(ClientManager::getInstance().countClients()) + " users, 0 services and 1 servers\r\n"
        + IRC_SERVER_NAME " 375 " + client->getUserInfo(Client::NICKNAME) + " :- server Message of the Day -\r\n"
        + IRC_SERVER_NAME " 372 " + client->getUserInfo(Client::NICKNAME) + " :-\n" + "                      /^--^\\     /^--^\\     /^--^\\\n" + "                      \\____/     \\____/     \\____/\n" + "                     /      \\   /      \\   /      \\\n" + "                    |        | |        | |        |\n" + "                     \\__  __/   \\__  __/   \\__  __/\n" + "|^|^|^|^|^|^|^|^|^|^|^|^\\ \\^|^|^|^/ /^|^|^|^|^\\ \\^|^|^|^|^|^|^|^|^|^|^|^|\n" + "| | | | | | | | | | | | |\\ \\| | |/ /| | | | | | \\ \\ | | | | | | | | | | |\n" + "| | | | | | | | | | | | / / | | |\\ \\| | | | | |/ /| | | | | | | | | | | |\n" + "| | | | | | | | | | | | \\/| | | | \\/| | | | | |\\/ | | | | | | | | | | | |\n" + "#########################################################################\n" + "| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |\n" + "| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |\r\n"
        + IRC_SERVER_NAME " 376 " + client->getUserInfo(Client::NICKNAME) + " :End of MOTD command\r\n");
        Utils::logDebug("Client " + Utils::toString(clientSocket) + " used " + commandName + " command");
    }
}

void Command::handleNick(int clientSocket, const std::vector<std::string>& args) {
    if (args.empty()) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 431 :No nickname given");
    }
    std::string nickname = args[0];

    // 닉네임 유효성 검사 (알파벳과 숫자만 허용)
    if (nickname.length() > 9 || nickname == "root" || nickname == "admin") {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 432 " + nickname + " :Erroneous nickname");
    }
    for (size_t i = 0; i < nickname.length(); ++i) {
        if (!isalnum(nickname[i])) {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 432 " + nickname + " :Erroneous nickname");
        }
    }

    // 닉네임 중복 검사
    if (ClientManager::getInstance().getClient(nickname) != NULL) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 433 " + nickname + " :Nickname is already in use");
    }

    Client* client = ClientManager::getInstance().getClient(clientSocket);
    if (!client) {
        throw IRCException::ClientFatalError("Client not found");
    }

    // 닉네임 설정 및 브로드캐스트
    std::string oldNick = client->getUserInfo(Client::NICKNAME);
    client->setUserInfo(Client::NICKNAME, nickname);
    if (!oldNick.empty()) {
        std::string message = client->generatePrefix() + " NICK :" + nickname + "\r\n";
        ChannelManager::getInstance().broadcastMessageAllChannels(message, -1);
    }
}

void Command::handleUser(int clientSocket, const std::vector<std::string>& args) {
    if (args.size() < 4) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 USER :Not enough parameters");
    }

    // 유효성 검사
    for (size_t i = 0; i < args[0].length(); ++i) {
        if (!isalnum(args[0][i])) {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 USER :Invalid username");
        }
    }

    Client* client = ClientManager::getInstance().getClient(clientSocket);
    if (!client) {
        throw IRCException::ClientFatalError("Client not found");
    }

    if (client->getRegistrationStep() != Client::USER) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 462 :Unauthorized command (already registered)");
    }

    client->setUserInfo(Client::USER_NAME, args[0]);
    client->setUserInfo(Client::HOST_NAME, args[1]);
    client->setUserInfo(Client::SERVER_NAME, args[2]);
    client->setUserInfo(Client::REAL_NAME, args[3]);
}

void Command::handleJoin(int clientSocket, const std::vector<std::string>& args) {
    if (args.empty()) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 JOIN :Not enough parameters");
    }

    std::string channelName = args[0];
	Client* client = ClientManager::getInstance().getClient(clientSocket);
    if (channelName[0] != '#') {
        //:irc.example.com 476 your_nick #bad_channel_name :Bad Channel Mask
        throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 476 " + client->getUserInfo(Client::NICKNAME) + " " + channelName + " :Bad Channel Mask");
    }
    std::string password = (args.size() > 1) ? args[1] : "";

    ChannelManager& channelManager = ChannelManager::getInstance();
    Channel* channel = channelManager.getChannel(channelName);

    if (!channel) {
        channelManager.createChannel(channelName, clientSocket);
        channel = channelManager.getChannel(channelName);
        if (!password.empty()) {
            channel->setInfo(Channel::PASSWORD, password);
        }
    } else if (channel->hasClientSocket(clientSocket)) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 443 "  " :You're already in the channel");
    }

    if (!channel->getInfo(Channel::PASSWORD).empty() && channel->getInfo(Channel::PASSWORD) != password) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 475 " + client->getUserInfo(Client::NICKNAME) + " " + channelName + " :Cannot join channel (Incorrect password)");
    }

    if (channel->getInfo(Channel::INVITE_ONLY) == "1" && !channel->isInviteSocket(clientSocket)) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 473 " + client->getUserInfo(Client::NICKNAME) + " " + channelName + " :Cannot join channel (Invite only)");
    }

    int limit = std::atoi(channel->getInfo(Channel::LIMIT).c_str());
    if (limit > 0 && channel->countClientSockets() >= limit) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 471 " + client->getUserInfo(Client::NICKNAME) + " " + channelName + " :Cannot join channel (Channel is full)");
    }

    channel->addClientSocket(clientSocket);
    channel->removeInviteSocket(clientSocket);

    // Send topic message if there is a topic
    if (!channel->getInfo(Channel::TOPIC).empty()) {
        ClientManager::getInstance().sendMessageToClient(clientSocket, IRC_SERVER_NAME " 332 " + client->getUserInfo(Client::NICKNAME) + " " + channelName + " :" + channel->getInfo(Channel::TOPIC) + "\r\n");
    } else {
        ClientManager::getInstance().sendMessageToClient(clientSocket, IRC_SERVER_NAME " 331 " + client->getUserInfo(Client::NICKNAME) + " " + channelName + " :No topic is set\r\n");
    }

    // Send names list
    std::ostringstream usersList;
    usersList << IRC_SERVER_NAME << " 353 " << client->getUserInfo(Client::NICKNAME) << " = " << channelName << " :";
    std::set<int> clients = channel->getClientSockets();
    for (std::set<int>::iterator it = clients.begin(); it != clients.end(); ++it) {
        Client* chClient = ClientManager::getInstance().getClient(*it);
        if (chClient) {
            if (channel->isOperatorSocket(chClient->getClientSocket())) {
                usersList << "@" << chClient->getUserInfo(Client::NICKNAME) << " ";
            } else {
                usersList << chClient->getUserInfo(Client::NICKNAME) << " ";
            }
        }
    }
    usersList << "\r\n";
    ClientManager::getInstance().sendMessageToClient(clientSocket, usersList.str());

    ClientManager::getInstance().sendMessageToClient(clientSocket, IRC_SERVER_NAME " 366 " + client->getUserInfo(Client::NICKNAME) + " " + channelName + " :End of /NAMES list\r\n");

    // Notify other clients about the new client in the channel
    std::string welcomeMessage = channel->isOperatorSocket(clientSocket) ? client->generatePrefix("@") + " JOIN " + channelName + "\r\n" : client->generatePrefix() + " JOIN " + channelName + "\r\n";
    channel->broadcastMessage(welcomeMessage, -1);
    // :" + permission + this->getUserInfo(Client::NICKNAME) + "!" + this->getUserInfo(Client::USER_NAME) + "@" + this->getUserInfo(Client::SERVER_NAME);
    
    std::time_t currentTime = std::time(NULL);
    std::tm* localTime = std::localtime(&currentTime);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
    client->sendMessage(":42bot!42bot@127.0.0.1 PRIVMSG " + channelName + " :welcome to 42 world by bonus today: " + buffer + "\r\n");
}

void Command::handlePart(int clientSocket, const std::vector<std::string>& args) {
    if (args.empty()) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 PART :Not enough parameters");
    }

    std::string channelName = args[0];
    Channel* channel = ChannelManager::getInstance().getChannel(channelName);
    Client* client = ClientManager::getInstance().getClient(clientSocket);

    if (!channel) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 403 " + channelName + " :No such channel");
    } else if (!channel->hasClientSocket(clientSocket)) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 442 " + channelName + " :You're not on that channel");
    } else {
        std::string partMessage = client->generatePrefix() + " PART " + channelName + "\r\n";
        channel->broadcastMessage(partMessage, -1);
        channel->removeClientSocket(clientSocket);
    }
}

void Command::handlePrivmsg(int clientSocket, const std::vector<std::string>& args) {
    if (args.size() < 2) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 PRIVMSG :Not enough parameters");
    }

    std::string recipient = args[0];
    std::string message;
    if (args.size() > 1) {
        for (size_t i = 1; i < args.size(); i++) {
            message += args[i];
            if (i < args.size() - 1) {
                message += " ";
            }
        }
    }
    Client* sender = ClientManager::getInstance().getClient(clientSocket);
    std::string fullMessage = sender->generatePrefix() + " PRIVMSG " + recipient + " " + message + "\r\n";

    if (recipient[0] == '#') {
        Channel* channel = ChannelManager::getInstance().getChannel(recipient);
        if (!channel) {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 403 " + recipient + " :No such channel");
        } else if (!channel->hasClientSocket(clientSocket)) {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 404 " + recipient + " :Cannot send to channel");
        }
        channel->broadcastMessage(fullMessage, clientSocket);
    } else {
        Client* targetClient = ClientManager::getInstance().getClient(recipient);
        if (!targetClient) {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 401 " + recipient + " :No such nick/channel");
        }
        targetClient->sendMessage(fullMessage);
    }
}

void Command::handleKick(int clientSocket, const std::vector<std::string>& args) {
    if (args.size() < 2) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 KICK :Not enough parameters");
    }

    std::string channelName = args[0];
    std::string userToKick = args[1];
    Channel* channel = ChannelManager::getInstance().getChannel(channelName);

    if (!channel) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 403 " + channelName + " :No such channel");
    }

    if (!channel->isOperatorSocket(clientSocket)) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 482 " + channelName + " :You're not channel operator");
    }

    Client* targetClient = ClientManager::getInstance().getClient(userToKick);
    if (!targetClient) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 401 " + ClientManager::getInstance().getClient(clientSocket)->getUserInfo(Client::NICKNAME) + " " + userToKick + " " + channelName + " :No such nick/channel");
    }

    if (targetClient->getClientSocket() == clientSocket) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 482 " + channelName + " :You cannot kick yourself");
    }

    if (channel->hasClientSocket(targetClient->getClientSocket())) {
        if (args.size() > 2) {
            // Kick with message
            std::string res;
            for (size_t i = 2; i < args.size(); ++i) {
                res += args[i];
                if (i < args.size() - 1) {
                    res += " ";
                }
            }
            std::string kickMessage = ClientManager::getInstance().getClient(clientSocket)->generatePrefix() + " KICK " + channelName + " " + userToKick + " " + res + "\r\n";
            channel->broadcastMessage(kickMessage, -1);
        } else {
            // Kick without message
            std::string kickMessage = ClientManager::getInstance().getClient(clientSocket)->generatePrefix() + " KICK " + channelName + " " + userToKick + "\r\n";
            channel->broadcastMessage(kickMessage, -1);
        }
        channel->removeClientSocket(targetClient->getClientSocket());
    } else {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 441 " + ClientManager::getInstance().getClient(clientSocket)->getUserInfo(Client::NICKNAME) + " " + userToKick + " " + channelName + " :They aren't on that channel");
    }
}

void Command::handleInvite(int clientSocket, const std::vector<std::string>& args) {
    if (args.size() < 2) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 INVITE :Not enough parameters");
    }

    std::string userToInvite = args[0];
    std::string channelName = args[1];
    Channel* channel = ChannelManager::getInstance().getChannel(channelName);

    if (!channel) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 403 " + channelName + " :No such channel");
    }

    if (!channel->isOperatorSocket(clientSocket)) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 482 " + channelName + " :You're not channel operator");
    }

    Client* inviteClient = ClientManager::getInstance().getClient(userToInvite);
    if (!inviteClient) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 401 " + userToInvite + " :No such nick/channel");
    }

    channel->addInviteSocket(inviteClient->getClientSocket());

    std::string inviteMessage = ClientManager::getInstance().getClient(clientSocket)->generatePrefix() + " INVITE " + userToInvite + " :" + channelName + "\r\n";
    inviteClient->sendMessage(inviteMessage);
    ClientManager::getInstance().sendMessageToClient(clientSocket, IRC_SERVER_NAME " 341 " + ClientManager::getInstance().getClient(clientSocket)->getUserInfo(Client::NICKNAME) + " " + userToInvite + " " + channelName + "\r\n");
}

void Command::handleTopic(int clientSocket, const std::vector<std::string>& args) {
    if (args.empty()) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 TOPIC :Not enough parameters");
    }

    std::string channelName = args[0];
    Channel* channel = ChannelManager::getInstance().getChannel(channelName);

    if (!channel) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 403 " + channelName + " :No such channel");
    }

	Client *client = ClientManager::getInstance().getClient(clientSocket);
    if (args.size() == 1) {
        // 주제 보기
        std::string topic = channel->getInfo(Channel::TOPIC);
        if (!topic.empty()) {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 332 " + client->getUserInfo(Client::NICKNAME) + " " + channelName + " :" + topic);
        } else {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 331 " + client->getUserInfo(Client::NICKNAME) + " " + channelName + " :No topic is set");
        }
		client->sendMessage(IRC_SERVER_NAME " 331 " + client->getUserInfo(Client::NICKNAME) + " " + channelName + " :No topic is set\r\n");	
    } else {
        // 주제 설정
        if (!channel->hasClientSocket(clientSocket)) {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 442 " + client->getUserInfo(Client::NICKNAME) + channelName + " :You're not on that channel");
        }
        bool isOperatorOnly = (channel->getInfo(Channel::TOPIC_PERMISSION) == "1");
        if (isOperatorOnly && !channel->isOperatorSocket(clientSocket)) {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 482 " + channelName + " :You're not channel operator");
        }
        std::string topic = args[1];
        channel->setInfo(Channel::TOPIC, topic);
        std::string topicMessage = client->generatePrefix() + " TOPIC " + channelName + " :" + topic + "\r\n";
        channel->broadcastMessage(topicMessage, -1);
    }
}

void Command::handleMode(int clientSocket, const std::vector<std::string>& args) {
    if (args.empty() || args.size() > 5) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 MODE :Not enough parameters");
		return;
	}

    std::string channelName = args[0];

    Channel* channel = ChannelManager::getInstance().getChannel(channelName);
	if (channelName[0] != '#' || args.size() == 1) {
		return ;
    } else if (!channel) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 403 " + channelName + " :No such channel");
    }

    if (!channel->isOperatorSocket(clientSocket)) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 482 " + channelName + " :You're not channel operator");
    }

	std::string modeChanges = args[1];
    std::string validModes = "+-iktol";
    bool adding = true;
    size_t argIndex = 2;
    for (size_t i = 0; i < modeChanges.size(); ++i) {
        char mode = modeChanges[i];
        if (mode == '+') {	
            adding = true;
        } else if (mode == '-') {
            adding = false;
        } else if (validModes.find(mode) == std::string::npos) {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 472 " + std::string(1, mode) + " :Unknown mode flag");
        } else {
            switch (mode) {
                case 'i': 
                    channel->setInfo(Channel::INVITE_ONLY, adding ? "1" : "0");
                    break;
                case 't':
                    channel->setInfo(Channel::TOPIC_PERMISSION, adding ? "1" : "0");
                    break;
                case 'k':
                    if (adding && argIndex < args.size()) {
                        channel->setInfo(Channel::PASSWORD, args[argIndex++]);
                    } else if (!adding) {
                        channel->setInfo(Channel::PASSWORD, "");
                    }
                    break;
                case 'o':
                    if (adding && argIndex < args.size()) {
                        Client* opClient = ClientManager::getInstance().getClient(args[argIndex++]);
                        if (opClient) {
                            channel->addOperatorSocket(opClient->getClientSocket());
                        }
                    } else if (!adding && argIndex < args.size()) {
                        Client* opClient = ClientManager::getInstance().getClient(args[argIndex++]);
                        if (opClient) {
                            channel->removeOperatorSocket(opClient->getClientSocket());
                        }
                    }
                    break;
                case 'l':
                    if (!adding) {
                        channel->setInfo(Channel::LIMIT, "0");
                    } else if (adding && argIndex < args.size()) {
                        for (size_t i = 0; i < args[argIndex].size(); i++) {
                            if (!std::isdigit(args[argIndex][i]))
                                throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 472 " + args[i] + " :Unknown mode flag");
                        }
                        channel->setInfo(Channel::LIMIT, args[argIndex++]); 
                    }
                    break;
            }
        }
    }
    std::string modeMessage = ClientManager::getInstance().getClient(clientSocket)->generatePrefix() + " MODE " + channelName + " " + modeChanges + "\r\n";
    channel->broadcastMessage(modeMessage, -1);
}

void Command::handleCap(int clientSocket, const std::vector<std::string>& args) {
    if (args.size() < 1) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 CAP :Not enough parameters");
    }
    std::string subCommand = args[0];
    std::ostringstream response;

    if (subCommand == "LS") {
        ClientManager::getInstance().sendMessageToClient(clientSocket, IRC_SERVER_NAME " CAP * LS :\r\n");
    } else if (subCommand == "REQ") {
        if (args.size() < 2) {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 CAP :Not enough parameters");
        }
        std::string reqParams = args[1];
        ClientManager::getInstance().sendMessageToClient(clientSocket, IRC_SERVER_NAME " CAP * NAK :" + reqParams + "\r\n");
    } else if (subCommand != "END") {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " CAP * ERROR :Unknown CAP subcommand");
    }
}

void Command::handlePing(int clientSocket, const std::vector<std::string>& args) {
    if (args.size() < 1) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 409 :No origin specified");
    }
    std::string response = IRC_SERVER_NAME " PONG " + args[0] + "\r\n";
    ClientManager::getInstance().sendMessageToClient(clientSocket, response);
}

void Command::handlePass(int clientSocket, const std::vector<std::string>& args) {
    if (ClientManager::getInstance().getClient(clientSocket)->getRegistrationStep() != Client::PASS) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 462 :Unauthorized command (already registered)");
    }
    if (args.empty()) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 PASS :Not enough parameters");
    }
    if (args[0] != IRCServer::getInstance().getServerPassword()) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 464 :Password incorrect");
    }
}

void Command::handleQuit(int clientSocket, const std::vector<std::string>& args) {
    Client* client = ClientManager::getInstance().getClient(clientSocket);
    if (!client) {
        throw IRCException::ClientFatalError("Client not found");
    }

    std::string quitMessage = client->generatePrefix() + " QUIT :" + (args.empty() ? "Client Quit" : &args[0][1]) + "\r\n";
    ChannelManager& channelManager = ChannelManager::getInstance();
    channelManager.broadcastMessageAllChannels(quitMessage, clientSocket);
    ClientManager::getInstance().deleteClient(clientSocket);
}

void Command::handleWho(int clientSocket, const std::vector<std::string>& args) {
    if (args.empty()) {
		throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 461 WHO :Not enough parameters");
    }

    std::string query = args[0];
    Client* requestingClient = ClientManager::getInstance().getClient(clientSocket);
    if (!requestingClient) {
        throw IRCException::ClientFatalError("Client not found");
    }

    // If query starts with '#', it's a channel
    if (query[0] == '#') {
        Channel* channel = ChannelManager::getInstance().getChannel(query);
        if (!channel) {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 403 " + query + " :No such channel");
        }

        // Iterate over channel's client sockets
        std::set<int> clientSockets = channel->getClientSockets();
        for (std::set<int>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it) {
            Client* client = ClientManager::getInstance().getClient(*it);
            if (client) {
                std::string prefix = client->generatePrefix();
                std::string whoMessage = IRC_SERVER_NAME " 352 " + requestingClient->getUserInfo(Client::NICKNAME) + " " + query + " " + client->getUserInfo(Client::USER_NAME) + " " + client->getUserInfo(Client::HOST_NAME) + " " + IRC_SERVER_NAME + " " + client->getUserInfo(Client::NICKNAME) + " H :0 " + client->getUserInfo(Client::REAL_NAME) + "\r\n";
                ClientManager::getInstance().sendMessageToClient(clientSocket, whoMessage);
            }
        }

        ClientManager::getInstance().sendMessageToClient(clientSocket, IRC_SERVER_NAME " 315 " + requestingClient->getUserInfo(Client::NICKNAME) + " " + query + " :End of /WHO list\r\n");
    } else {
        // Query is a nickname
        Client* client = ClientManager::getInstance().getClient(query);
        if (!client) {
			throw IRCException::ClientGeneralError(IRC_SERVER_NAME " 401 " + query + " :No such nick/channel");
        }

        std::string prefix = client->generatePrefix();
        std::string whoMessage = IRC_SERVER_NAME " 352 " + requestingClient->getUserInfo(Client::NICKNAME) + " * " + client->getUserInfo(Client::USER_NAME) + " " + client->getUserInfo(Client::HOST_NAME) + " " + IRC_SERVER_NAME + " " + client->getUserInfo(Client::NICKNAME) + " H :0 " + client->getUserInfo(Client::REAL_NAME) + "\r\n";
        ClientManager::getInstance().sendMessageToClient(clientSocket, whoMessage);

        ClientManager::getInstance().sendMessageToClient(clientSocket, IRC_SERVER_NAME " 315 " + requestingClient->getUserInfo(Client::NICKNAME) + " " + query + " :End of /WHO list\r\n");
    }
}

Command::CommandHandler Command::getCommand(const std::string& commandName) {
    std::map<std::string, Command::CommandHandler>::iterator it = this->commands.find(commandName);
    if (it != this->commands.end()) {
        return it->second;
    }
    return 0;
}
