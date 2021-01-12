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
		const std::string ip;
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
		static int sendBytes(SOCKET clientSocket, const DataT* const data, int count);

		template<typename DataT>
		static int receiveBytes(SOCKET clientSocket, DataT* const data, int count);

	protected:
		static std::string getIpV4(sockaddr& addr);

	public:
		template<typename PortStringT, typename IPStringT = std::string>
		BaseTCPServer(const PortStringT& port, const IPStringT& ip = "0.0.0.0", DWORD timeout = 0, bool freeDLL = true);

		virtual void start();

		virtual void stop();

		virtual bool serverState() const;

		virtual void pubDisconnect(const std::string& ip) final;

		virtual std::vector<std::pair<std::string, SOCKET>> getClients() final;

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
	BaseTCPServer::BaseTCPServer(const PortStringT& port, const IPStringT& ip, DWORD timeout, bool freeDLL) :
		port(std::string(port)),
		ip(std::string(ip)),
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

		if (getaddrinfo(this->ip.data(), this->port.data(), &hints, &info))
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