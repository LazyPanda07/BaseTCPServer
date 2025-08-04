#include "BaseTCPServer.h"

#include <iostream>

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
	void BaseTCPServer::ClientData::add(const string& ip, SOCKET socket)
	{
		unique_lock<mutex> lock(dataMutex);

		data[ip].push_back(socket);
	}

	void BaseTCPServer::ClientData::remove(const string& ip, SOCKET socket)
	{
		unique_lock<mutex> lock(dataMutex);

		if (!data.contains(ip))
		{
			return;
		}

		erase(data[ip], socket);
	}

	vector<SOCKET> BaseTCPServer::ClientData::extract(const string& ip)
	{
		unique_lock<mutex> lock(dataMutex);
		vector<SOCKET> result;

		if (auto node = data.extract(ip))
		{
			result = move(node.mapped());
		}

		return result;
	}

	void BaseTCPServer::ClientData::clear()
	{
		unique_lock<mutex> lock(dataMutex);

		data.clear();
	}

	vector<pair<string, vector<SOCKET>>> BaseTCPServer::ClientData::getClients() const
	{
		vector<pair<string, vector<SOCKET>>> result;
		unique_lock<mutex> lock(dataMutex);

		result.reserve(data.size());

		for (const auto& [ip, sockets] : data)
		{
			result.emplace_back(ip, sockets);
		}

		return result;
	}

	size_t BaseTCPServer::ClientData::getNumberOfClients() const
	{
		unique_lock<mutex> lock(dataMutex);

		return data.size();
	}

	size_t BaseTCPServer::ClientData::getNumberOfConnections() const
	{
		size_t result = 0;

		unique_lock<mutex> lock(dataMutex);

		for (const auto& [key, value] : data)
		{
			result += value.size();
		}

		return result;
	}

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
			THROW_WEB_SERVER_EXCEPTION;
		}

		if ((listenSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == INVALID_SOCKET)
		{
			freeaddrinfo(info);

			freeDLL = true;

			THROW_WEB_SERVER_EXCEPTION;
		}

#ifdef __LINUX__
		int flags = fcntl(listenSocket, F_GETFL, 0);

		if (flags == -1)
		{
			cerr << "Can't F_GETFL on listen socket" << endl;

			flags = 0;
		}

		flags = this->isListenSocketInBlockingMode() ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);

		if (fcntl(listenSocket, F_SETFL, flags) == SOCKET_ERROR)
		{
			THROW_WEB_SERVER_EXCEPTION;
		}
#else
		if (ioctlsocket(listenSocket, FIONBIO, &listenSocketBlockingMode) == SOCKET_ERROR)
		{
			THROW_WEB_SERVER_EXCEPTION;
		}
#endif

		if (::bind(listenSocket, info->ai_addr, static_cast<int>(info->ai_addrlen)) == SOCKET_ERROR)
		{
			freeaddrinfo(info);

			freeDLL = true;

			THROW_WEB_SERVER_EXCEPTION;
		}

		if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			freeaddrinfo(info);

			freeDLL = true;

			THROW_WEB_SERVER_EXCEPTION;
		}

		freeaddrinfo(info);
	}

	void BaseTCPServer::receiveConnections(const function<void()>& onStartServer, exception** outException)
	{
		try
		{
#ifdef __LINUX__
			socklen_t addrlen = sizeof(sockaddr);
#else
			int addrlen = sizeof(sockaddr);
#endif
			if (onStartServer)
			{
				onStartServer();
			}

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
						THROW_WEB_SERVER_EXCEPTION;
					}

					if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutValue), sizeof(timeoutValue)) == SOCKET_ERROR)
					{
						THROW_WEB_SERVER_EXCEPTION;
					}

#ifdef __LINUX__
					int flags = fcntl(clientSocket, F_GETFL, 0);

					if (flags == -1)
					{
						cerr << "Can't F_GETFL on socket" << endl;

						flags = 0;
					}

					flags = this->isAcceptedSocketsInBlockingMode() ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);

					if (fcntl(clientSocket, F_SETFL, flags) == SOCKET_ERROR)
					{
						THROW_WEB_SERVER_EXCEPTION;
					}
#else
					if (ioctlsocket(clientSocket, FIONBIO, &blockingMode) == SOCKET_ERROR)
					{
						THROW_WEB_SERVER_EXCEPTION;
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
				else
				{
					this->onInvalidConnectionReceive();
				}
			}

			if (this->getNumberOfConnections())
			{
				this->kickAll();
			}
		}
		catch (const exception& e)
		{
			if (outException)
			{
				*outException = new runtime_error(e.what());
			}
			else
			{
				cerr << __func__ << " throws exception: " << e.what() << endl;
			}
		}
	}

	void BaseTCPServer::serve(string ip, SOCKET clientSocket, sockaddr address)
	{
		function<void()> cleanup = [this, clientSocket, ip]()
			{
				closesocket(clientSocket);

				data.remove(ip, clientSocket);
			};

		this->onConnectionReceive(clientSocket, address);

		this->clientConnection(ip, clientSocket, address, cleanup);

		if (static_cast<bool>(cleanup))
		{
			cleanup();
		}
	}

	void BaseTCPServer::onConnectionReceive(SOCKET clientSocket, sockaddr address)
	{

	}

	void BaseTCPServer::onInvalidConnectionReceive()
	{

	}

	string BaseTCPServer::getClientIpV4(sockaddr address)
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

	uint16_t BaseTCPServer::getClientPortV4(sockaddr address)
	{
		return ntohs(reinterpret_cast<const sockaddr_in&>(address).sin_port);
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

	string BaseTCPServer::getVersion()
	{
		string version = "1.14.0";

		return version;
	}

	BaseTCPServer::BaseTCPServer(string_view port, string_view ip, DWORD timeout, bool multiThreading, u_long listenSocketBlockingMode, bool freeDLL) :
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
			THROW_WEB_SERVER_EXCEPTION;
		}
#endif // __LINUX__
	}

	void BaseTCPServer::start(bool wait, const function<void()>& onStartServer, exception** outException)
	{
		this->createListenSocket();

		isRunning = true;

		handle = async(launch::async, &BaseTCPServer::receiveConnections, this, onStartServer, outException);

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

	void BaseTCPServer::kick(const string& ip)
	{
		vector<SOCKET> sockets = data.extract(ip);

		for (SOCKET socket : sockets)
		{
			closesocket(socket);
		}
	}

	void BaseTCPServer::kickAll()
	{
		for (auto&& [ip, sockets] : data.getClients())
		{
			for (SOCKET socket : sockets)
			{
				closesocket(socket);
			}
		}

		data.clear();
	}

	bool BaseTCPServer::isServerRunning() const
	{
		return isRunning;
	}

	bool BaseTCPServer::isListenSocketInBlockingMode() const
	{
		return !static_cast<bool>(listenSocketBlockingMode);
	}

	bool BaseTCPServer::isAcceptedSocketsInBlockingMode() const
	{
		return !static_cast<bool>(blockingMode);
	}

	void BaseTCPServer::setAcceptedSocketsBlockingMode(bool block)
	{
		blockingMode = !block;
	}

	size_t BaseTCPServer::getNumberOfClients() const
	{
		return data.getNumberOfClients();
	}

	size_t BaseTCPServer::getNumberOfConnections() const
	{
		return data.getNumberOfConnections();
	}

	vector<pair<string, vector<SOCKET>>> BaseTCPServer::getClients() const
	{
		return data.getClients();
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
