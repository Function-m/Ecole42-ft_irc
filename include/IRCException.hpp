#ifndef IRCEXCEPTION_HPP
#define IRCEXCEPTION_HPP

#include <stdexcept>
#include <string>

class IRCException {
   public:
	class ServerFatalError : public std::runtime_error {
	   public:
		explicit ServerFatalError(const std::string& message) : std::runtime_error(message) {}
		virtual ~ServerFatalError() throw() {}
	};
	class ServerGeneralError : public std::runtime_error {
	   public:
		explicit ServerGeneralError(const std::string& message) : std::runtime_error(message) {}
		virtual ~ServerGeneralError() throw() {}
	};
	class ClientFatalError : public std::runtime_error {
	   public:
		explicit ClientFatalError(const std::string& message) : std::runtime_error(message) {}
		virtual ~ClientFatalError() throw() {}
	};
	class ClientGeneralError : public std::runtime_error {
	   public:
		explicit ClientGeneralError(const std::string& message) : std::runtime_error(message) {}
		virtual ~ClientGeneralError() throw() {}
	};
};

#endif	// IRCEXCEPTION_HPP