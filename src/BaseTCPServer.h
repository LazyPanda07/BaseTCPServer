#pragma once

#include <string>
#include <future>

#ifdef __LINUX__
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#else
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif

#include "WebException.h"
#include "ClientData.h"

namespace web
{
	class BaseTCPServer
	{
	public:
		static constexpr size_t ipV4Size = 16;

	protected:
		ClientData data;
		std::string ip;
		std::string port;
		SOCKET listenSocket;
		u_long blockingMode;
		u_long listenSocketBlockingMode;
		DWORD timeout;
		bool freeDLL;
		bool isRunning;
		bool multiThreading;
		std::future<void> handle;

	protected:
		void createListenSocket();

		virtual void receiveConnections();

		virtual void serve(std::string ip, SOCKET clientSocket, sockaddr address);

		virtual void clientConnection(SOCKET clientSocket, const sockaddr& address) = 0;

		virtual void onConnectionReceive(SOCKET clientSocket, const sockaddr& address);
		
	protected:
		template<typename DataT>
		static int sendBytes(SOCKET clientSocket, const DataT* const data, int count);

		template<typename DataT>
		static int receiveBytes(SOCKET clientSocket, DataT* const data, int count);

	public:
		static std::string getClientIpV4(const sockaddr& address);

		std::string getServerIpV4() const;

		static uint16_t getClientPortV4(sockaddr& address);

		uint16_t getServerPortV4() const;

	public:
		/// @brief 
		/// @param port Server's port
		/// @param ip Server's ip
		/// @param timeout recv function timeout in milliseconds, 0 wait for upcoming data
		/// @param multiThreading Each client in separate thread
		/// @param listenSocketBlockingMode Blocking mode for listen socket (0 - blocking, non 0 - non blocking)
		/// @param freeDLL Unload Ws2_32.dll in destructor(Windows only parameter)
		BaseTCPServer(const std::string& port, const std::string& ip = "0.0.0.0", DWORD timeout = 0, bool multiThreading = true, u_long listenSocketBlockingMode = 0, bool freeDLL = true);

		/**
		 * @brief Start server in separate thread
		 * @param wait Wait server serving in current thread
		 */
		void start(bool wait = false);

		/**
		 * @brief Stop receiving new connections
		 * @param wait Wait all clients tasks
		 */
		void stop(bool wait = true);

		/**
		 * @brief Is server accept new connections
		 * @return 
		 */
		bool isServerRunning() const;

		/**
		 * @brief Number of IP addresses
		 * @return 
		 */
		size_t getNumberOfClients() const;

		/**
		 * @brief Number of sockets(each IP address may have few sockets)
		 * @return 
		 */
		size_t getNumberOfConnections() const;

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

			if (lastSend == SOCKET_ERROR)
			{
				THROW_WEB_EXCEPTION;
			}
			else if (!lastSend)
			{
				return totalSent;
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

			if (lastReceive == SOCKET_ERROR)
			{
				THROW_WEB_EXCEPTION;
			}
			else if (!lastReceive)
			{
				return totalReceive;
			}

			totalReceive += lastReceive;

		} while (totalReceive < count);

		return totalReceive;
	}
}
