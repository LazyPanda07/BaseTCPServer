#pragma once

#include <string>

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "WebException.h"

namespace web
{
	class BaseTCPServer
	{
	protected:
		SOCKET listenSocket;

		const std::string port;
		bool freeDLL;
		bool isRunning;

	protected:
		virtual void receiveConnections();

		virtual void clientConnection(SOCKET clientSocket, sockaddr addr) = 0;

	protected:
		template<typename DataT>
		static __int32 sendBytes(SOCKET clientSocket, const DataT* const data, __int32 count);

		template<typename DataT>
		static __int32 receiveBytes(SOCKET clientSocket, DataT* const data, __int32 count);

	public:
		template<typename StringT>
		BaseTCPServer(const StringT& port, bool freeDLL = true);

		virtual void start();

		virtual void stop();

		virtual bool serverState() const;

		virtual ~BaseTCPServer();
	};

	template<typename DataT>
	__int32 BaseTCPServer::sendBytes(SOCKET clientSocket, const DataT* const data, __int32 count)
	{
		__int32  lastSend = 0;
		__int32 totalSend = 0;

		do
		{
			lastSend = send(clientSocket, reinterpret_cast<const char*>(data) + totalSend, count - totalSend, NULL);

			if (lastSend <= 0)
			{
				throw WebException();
			}

			totalSend += lastSend;

		} while (totalSend < count);

		return totalSend;
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
	BaseTCPServer::BaseTCPServer(const StringT& port, bool freeDLL) : port(std::string(port)), freeDLL(freeDLL)
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