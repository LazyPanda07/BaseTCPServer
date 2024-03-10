#include "BaseTCPServer.h"

#include <iostream>
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
		if (listenSocketBlockingMode && fcntl(listenSocket, F_SETFL, O_NONBLOCK) == SOCKET_ERROR)
		{
			THROW_WEB_EXCEPTION;
		}
#else
		if (listenSocketBlockingMode && ioctlsocket(listenSocket, FIONBIO, &listenSocketBlockingMode) == SOCKET_ERROR)
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
			sockaddr address;
			SOCKET clientSocket = accept(listenSocket, &address, &addrlen);

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
				if (blockingMode && fcntl(clientSocket, F_SETFL, O_NONBLOCK) == SOCKET_ERROR)
				{
					THROW_WEB_EXCEPTION;
				}
#else
				if (blockingMode && ioctlsocket(clientSocket, FIONBIO, &blockingMode) == SOCKET_ERROR)
				{
					THROW_WEB_EXCEPTION;
				}
#endif

				string ip = BaseTCPServer::getClientIpV4(address);

				data.add(ip, clientSocket);

				if (multiThreading)
				{
					thread(&BaseTCPServer::serve, this, ip, clientSocket, address).detach();
				}
				else
				{
					this->serve(ip, clientSocket, address);
				}
			}
		}

		while (true)
		{
			if (!this->getNumberOfConnections())
			{
				break;
			}

			this_thread::sleep_for(1s);
		}
	}

	void BaseTCPServer::serve(string ip, SOCKET clientSocket, sockaddr address)
	{
		this->onConnectionReceive(clientSocket, address);

		this->clientConnection(clientSocket, address);

		closesocket(clientSocket);

		data.remove(ip, clientSocket);
	}

	void BaseTCPServer::onConnectionReceive(SOCKET clientSocket, const sockaddr& address)
	{

	}

	string BaseTCPServer::getClientIpV4(const sockaddr& address)
	{
		string ip(BaseTCPServer::ipV4Size, '\0');

		inet_ntop(AF_INET, reinterpret_cast<const char*>(&reinterpret_cast<const sockaddr_in*>(&address)->sin_addr), ip.data(), BaseTCPServer::ipV4Size);

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

		ip.resize(BaseTCPServer::ipV4Size);

		getsockname(listenSocket, reinterpret_cast<sockaddr*>(&serverInfo), &len);

		inet_ntop(AF_INET, &serverInfo.sin_addr, ip.data(), BaseTCPServer::ipV4Size);

		while (ip.back() == '\0')
		{
			ip.pop_back();
		}

		return ip;
	}

	uint16_t BaseTCPServer::getClientPortV4(sockaddr& address)
	{
		return ntohs(reinterpret_cast<sockaddr_in&>(address).sin_port);
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

	bool BaseTCPServer::isServerRunning() const
	{
		return isRunning;
	}

	size_t BaseTCPServer::getNumberOfClients() const
	{
		return data.getNumberOfClients();
	}

	size_t BaseTCPServer::getNumberOfConnections() const
	{
		return data.getNumberOfConnections();
	}

	BaseTCPServer::~BaseTCPServer()
	{
		if (isRunning)
		{
			this->stop();
		}

#ifndef __LINUX__
		if (freeDLL)
		{
			WSACleanup();
		}
#endif
	}
}
