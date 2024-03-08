#include "BaseTCPServer.h"

#include <thread>

#ifdef __LINUX__
#include <fcntl.h>
#include <arpa/inet.h>
#endif

#ifndef __LINUX__
#pragma comment (lib,"ws2_32.lib")
#endif

using namespace std;

namespace web
{
	void BaseTCPServer::createListenSocket()
	{
		addrinfo* info = nullptr;
		addrinfo hints = {};

		hints.ai_family = AF_INET;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_socktype = SOCK_STREAM;

		if (getaddrinfo(ip.data(), port.data(), &hints, &info))
		{
			THROW_WEB_EXCEPTION;
		}

		if ((listenSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == INVALID_SOCKET)
		{
			freeaddrinfo(info);

			freeDLL = true;

			THROW_WEB_EXCEPTION;
		}

#ifdef __LINUX__
		if (fcntl(listenSocket, F_SETFL,(listenSocketBlockingMode ? ~O_NONBLOCK : O_NONBLOCK)) == SOCKET_ERROR)
		{
			THROW_WEB_EXCEPTION;
		}
#else
		if (ioctlsocket(listenSocket, FIONBIO, &listenSocketBlockingMode) == SOCKET_ERROR)
		{
			THROW_WEB_EXCEPTION;
		}
#endif

		if (::bind(listenSocket, info->ai_addr, static_cast<int>(info->ai_addrlen)) == SOCKET_ERROR)
		{
			freeaddrinfo(info);

			freeDLL = true;

			THROW_WEB_EXCEPTION;
		}

		if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			freeaddrinfo(info);

			freeDLL = true;

			THROW_WEB_EXCEPTION;
		}

		freeaddrinfo(info);
	}

	void BaseTCPServer::receiveConnections()
	{
#ifdef __LINUX__
		socklen_t addrlen = sizeof(sockaddr);
#else
		int addrlen = sizeof(sockaddr);
#endif
		
		while (isRunning)
		{
			sockaddr addr;
			SOCKET clientSocket = accept(listenSocket, &addr, &addrlen);

#ifdef __LINUX__
			timeval timeoutValue;

			timeoutValue.tv_sec = timeout / 1000;
			timeoutValue.tv_usec = (timeout - timeoutValue.tv_sec * 1000) * 1000;
#else
			DWORD timeoutValue = timeout;
#endif

			if (isRunning && clientSocket != INVALID_SOCKET)
			{
				if (setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeoutValue), sizeof(timeoutValue)) == SOCKET_ERROR)
				{
					THROW_WEB_EXCEPTION;
				}

				if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutValue), sizeof(timeoutValue)) == SOCKET_ERROR)
				{
					THROW_WEB_EXCEPTION;
				}

#ifdef __LINUX__
				if (fcntl(clientSocket, F_SETFL, (blockingMode ? ~O_NONBLOCK : O_NONBLOCK)) == SOCKET_ERROR)
				{
					THROW_WEB_EXCEPTION;
				}
#else
				if (ioctlsocket(clientSocket, FIONBIO, &blockingMode) == SOCKET_ERROR)
				{
					THROW_WEB_EXCEPTION;
				}
#endif
				
				data.insert(getClientIpV4(addr), clientSocket);

				if (multiThreading)
				{
					thread(&BaseTCPServer::clientConnection, this, clientSocket, addr).detach();
				}
				else
				{
					this->clientConnection(clientSocket, addr);
				}

				this->onConnectionReceive(clientSocket, addr);
			}
		}

		for (const auto& [ip, _] : data.getClients())
		{
			this->pubDisconnect(ip);
		}
	}

	void BaseTCPServer::disconnect(const string& ip)
	{
		try
		{
			vector<SOCKET> sockets = data[ip];

			for (SOCKET socket : sockets)
			{
				this->onDisconnect(socket, ip);

				closesocket(socket);
			}
		}
		catch (const exception&)
		{

		}
	}

	void BaseTCPServer::onConnectionReceive(SOCKET clientSocket, sockaddr addr)
	{

	}

	void BaseTCPServer::onDisconnect(SOCKET clientSocket, const string& ip)
	{

	}

	string BaseTCPServer::getClientIpV4(sockaddr& addr)
	{
		string ip;

		ip.resize(16);

		inet_ntop(AF_INET, reinterpret_cast<const char*>(&reinterpret_cast<sockaddr_in*>(&addr)->sin_addr), ip.data(), ip.size());

		while (ip.back() == '\0')
		{
			ip.pop_back();
		}

		return ip;
	}

	string BaseTCPServer::getServerIpV4() const
	{
		string ip;
		sockaddr_in serverInfo = {};

#ifdef __LINUX__
		socklen_t len = sizeof(serverInfo);
#else
		int len = sizeof(serverInfo);
#endif
		
		ip.resize(16);

		getsockname(listenSocket, reinterpret_cast<sockaddr*>(&serverInfo), &len);

		inet_ntop(AF_INET, &serverInfo.sin_addr, ip.data(), ip.size());

		while (ip.back() == '\0')
		{
			ip.pop_back();
		}

		return ip;
	}

	uint16_t BaseTCPServer::getClientPortV4(sockaddr& addr)
	{
		return ntohs(reinterpret_cast<sockaddr_in&>(addr).sin_port);
	}

	uint16_t BaseTCPServer::getServerPortV4() const
	{
		sockaddr_in serverInfo = {};

#ifdef __LINUX__
		socklen_t len = sizeof(serverInfo);
#else
		int len = sizeof(serverInfo);
#endif
		
		getsockname(listenSocket, reinterpret_cast<sockaddr*>(&serverInfo), &len);

		return ntohs(serverInfo.sin_port);
	}

	BaseTCPServer::BaseTCPServer(const string& port, const string& ip, DWORD timeout, bool multiThreading, u_long listenSocketBlockingMode, bool freeDLL) :
		ip(ip),
		port(port),
		listenSocket(INVALID_SOCKET),
		blockingMode(0),
		listenSocketBlockingMode(listenSocketBlockingMode),
		timeout(timeout),
		freeDLL(freeDLL),
		isRunning(false),
		multiThreading(multiThreading)
	{
#ifndef __LINUX__
		WSADATA wsaData;

		if (WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			THROW_WEB_EXCEPTION;
		}
#endif // __LINUX__
	}

	void BaseTCPServer::start(bool wait)
	{
		this->createListenSocket();

		isRunning = true;

		handle = async(launch::async, &BaseTCPServer::receiveConnections, this);

		if (wait)
		{
			handle.wait();
		}
	}

	void BaseTCPServer::stop(bool wait)
	{
		isRunning = false;

		closesocket(listenSocket);

		if (wait)
		{
			handle.wait();
		}
	}

	bool BaseTCPServer::serverState() const
	{
		return isRunning;
	}

	void BaseTCPServer::pubDisconnect(const string& ip)
	{
		this->disconnect(ip);

		data.erase(ip);
	}

	vector<pair<string, SOCKET>> BaseTCPServer::getClients()
	{
		return data.getClients();
	}

	void BaseTCPServer::setBlockingModeForOtherConnections(u_long blockingMode)
	{
		this->blockingMode = blockingMode;
	}

	u_long BaseTCPServer::blockingModeForOtherConnections() const
	{
		return blockingMode;
	}

	BaseTCPServer::~BaseTCPServer()
	{
		isRunning = false;

#ifndef __LINUX__
		if (freeDLL)
		{
			WSACleanup();
		}
#endif
	}
}
