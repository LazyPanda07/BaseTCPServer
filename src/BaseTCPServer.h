#pragma once

#include <string>

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "WebException.h"
#include "ClientData.h"

namespace web
{
	class BaseTCPServer
	{
	protected:
		ClientData data;
		
		const std::string port;
		
		SOCKET listenSocket;

		DWORD timeout;

		bool freeDLL;
		bool isRunning;

	protected:
		virtual void receiveConnections();

		virtual void disconnect(const std::string& ip);

		virtual void clientConnection(SOCKET clientSocket, sockaddr addr) = 0;

	protected:
		template<typename DataT>
		static __int32 sendBytes(SOCKET clientSocket, const DataT* const data, __int32 count);

		template<typename DataT>
		static __int32 receiveBytes(SOCKET clientSocket, DataT* const data, __int32 count);

	protected:
		static std::string getIpV4(sockaddr& addr);

	public:
		template<typename StringT>
		BaseTCPServer(const StringT& port, DWORD timeout = 0, bool freeDLL = true);

		virtual void start();

		virtual void stop();

		virtual bool serverState() const;

		virtual void pubDisconnect(const std::string& ip) final;

		virtual std::vector<std::pair<std::string, SOCKET>> getClients() final;

		virtual ~BaseTCPServer();
	};

	template<typename DataT>
	__int32 BaseTCPServer::sendBytes(SOCKET clientSocket, const DataT* const data, __int32 count)
	{
		__int32  lastSend = 0;
		__int32 totalSent = 0;

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
	__int32 BaseTCPServer::receiveBytes(SOCKET clientSocket, DataT* const data, __int32 count)
	{
		__int32 lastReceive = 0;
		__int32 totalReceive = 0;

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

	template<typename StringT>
	BaseTCPServer::BaseTCPServer(const StringT& port, DWORD timeout, bool freeDLL) :
		port(std::string(port)),
		freeDLL(freeDLL),
		isRunning(false),
		timeout(timeout)
	{
		WSADATA wsaData;
		addrinfo* info;
		addrinfo hints = {};

		if (WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			throw WebException();
		}

		hints.ai_family = AF_INET;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_socktype = SOCK_STREAM;

		if (getaddrinfo(nullptr, port.data(), &hints, &info))
		{
			throw WebException();
		}

		if ((listenSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == INVALID_SOCKET)
		{
			throw WebException();
		}

		if (bind(listenSocket, info->ai_addr, info->ai_addrlen) == SOCKET_ERROR)
		{
			throw WebException();
		}

		if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			throw WebException();
		}

		freeaddrinfo(info);
	}
}