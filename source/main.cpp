#include "ChannelManager.hpp"
#include "ClientManager.hpp"
#include "IRCServer.hpp"
#include "Utils.hpp"
#include "IRCException.hpp"

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return EXIT_FAILURE;
	}

	try {
		// Run the server
		IRCServer::getInstance().run(std::atoi(argv[1]), argv[2]);

		// Stop the server
		IRCServer::getInstance().~IRCServer();
		ClientManager::getInstance().~ClientManager();
		ChannelManager::getInstance().~ChannelManager();
		Utils::logInfo("Server stopped");

	} catch (const IRCException::ServerFatalError& e) {
		std::cerr << "Fatal exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	} catch (const std::exception& e) {
		std::cerr << "Fatal exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	} catch (...) {
		std::cerr << "Unknown fatal exception" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
