#pragma once

#include <string>

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "WebException.h"
#include "ClientData.h"

using namespace std::string_view_literals;

namespace web
{
	class BaseTCPServer
	{
	protected:
		ClientData data;
		SOCKET listenSocket;
		u_long blockingMode;
		DWORD timeout;
		bool freeDLL;
		bool isRunning;
		bool multiThreading;

	protected:
		virtual void receiveConnections();

		virtual void disconnect(const std::string& ip);

		virtual void clientConnection(SOCKET clientSocket, sockaddr addr) = 0;

	protected:
		template<typename DataT>
		static int sendBytes(SOCKET clientSocket, const DataT* const data, int count);

		template<typename DataT>
		static int receiveBytes(SOCKET clientSocket, DataT* const data, int count);

	public:
		static std::string getClientIpV4(sockaddr& addr);

		std::string getServerIpV4() const;

		static uint16_t getClientPortV4(sockaddr& addr);

		uint16_t getServerPortV4() const;

	public:
		/// @brief 
		/// @tparam PortStringT 
		/// @tparam IPStringT 
		/// @param port Server's port
		/// @param ip Server's ip
		/// @param timeout recv function timeout in milliseconds, 0 wait for upcoming data
		/// @param multiThreading Each client in separate thread
		/// @param listenerSocketBlockingMode Blocking mode for listener socket (0 - blocking, not 0 - non blocking)
		/// @param freeDLL Unload Ws2_32.dll in destructor
		template<typename PortStringT, typename IPStringT = std::string_view>
		BaseTCPServer(const PortStringT& port, const IPStringT& ip = "0.0.0.0"sv, DWORD timeout = 0, bool multiThreading = true, u_long listenerSocketBlockingMode = 0, bool freeDLL = true);

		virtual void start();

		virtual void stop();

		virtual bool serverState() const;

		virtual void pubDisconnect(const std::string& ip) final;

		virtual std::vector<std::pair<std::string, SOCKET>> getClients() final;

		u_long& blockingModeForOtherConnections();

		const u_long& blockingModeForOtherConnections() const;

		virtual ~BaseTCPServer();
	};

	template<typename DataT>
	int BaseTCPServer::sendBytes(SOCKET clientSocket, const DataT* const data, int count)
	{
		int lastSend = 0;
		int totalSent = 0;

		do
		{
			lastSend = send(clientSocket, reinterpret_cast<const char*>(data) + totalSent, count - totalSent, NULL);

			if (lastSend <= 0)
			{
				throw WebException();
			}

			totalSent += lastSend;

		} while (totalSent < count);

		return totalSent;
	}

	template<typename DataT>
	int BaseTCPServer::receiveBytes(SOCKET clientSocket, DataT* const data, int count)
	{
		int lastReceive = 0;
		int totalReceive = 0;

		do
		{
			lastReceive = recv(clientSocket, reinterpret_cast<char*>(data) + totalReceive, count - totalReceive, NULL);

			if (lastReceive <= 0)
			{
				throw WebException();
			}

			totalReceive += lastReceive;

		} while (totalReceive < count);

		return totalReceive;
	}

	template<typename PortStringT, typename IPStringT>
	BaseTCPServer::BaseTCPServer(const PortStringT& port, const IPStringT& ip, DWORD timeout, bool multiThreading, u_long listenerSocketBlockingMode, bool freeDLL) :
		blockingMode(0),
		timeout(timeout),
		freeDLL(freeDLL),
		isRunning(false),
		multiThreading(multiThreading)
	{
		WSADATA wsaData;
		addrinfo* info = nullptr;
		addrinfo hints = {};

		if (WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			throw WebException();
		}

		hints.ai_family = AF_INET;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_socktype = SOCK_STREAM;

		if (getaddrinfo(ip.data(), port.data(), &hints, &info))
		{
			WSACleanup();

			throw WebException();
		}

		if ((listenSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == INVALID_SOCKET)
		{
			freeaddrinfo(info);

			WSACleanup();

			throw WebException();
		}

		ioctlsocket(listenSocket, FIONBIO, &listenerSocketBlockingMode);

		if (bind(listenSocket, info->ai_addr, info->ai_addrlen) == SOCKET_ERROR)
		{
			freeaddrinfo(info);

			WSACleanup();

			throw WebException();
		}

		if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			freeaddrinfo(info);

			WSACleanup();

			throw WebException();
		}

		freeaddrinfo(info);
	}
}
