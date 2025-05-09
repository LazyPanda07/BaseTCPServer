#pragma once

#include <string>
#include <future>
#include <unordered_map>
#include <vector>

#ifdef __LINUX__
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#else
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif // __LINUX__

#include "WebException.h"

#ifdef __LINUX__
#ifndef WINDOWS_STYLE_DEFINITION
#define WINDOWS_STYLE_DEFINITION

#define closesocket close
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define DWORD uint32_t

#endif // WINDOWS_STYLE_DEFINITION
#endif // __LINUX__

namespace web
{
	class BaseTCPServer
	{
	private:
		class ClientData
		{
		private:
			std::unordered_map<std::string, std::vector<SOCKET>> data;
			mutable std::mutex dataMutex;

		public:
			ClientData() = default;

			void add(const std::string& ip, SOCKET socket);

			void remove(const std::string& ip, SOCKET socket);

			std::vector<SOCKET> extract(const std::string& ip);

			void clear();

			std::vector<std::pair<std::string, std::vector<SOCKET>>> getClients() const;

			size_t getNumberOfClients() const;

			size_t getNumberOfConnections() const;

			~ClientData() = default;
		};

	public:
		static constexpr size_t ipV4Size = 16;

	protected:
		ClientData data;
		std::string ip;
		std::string port;
		SOCKET listenSocket;
		DWORD timeout;
		bool freeDLL;
		bool isRunning;
		const bool multiThreading;
		std::future<void> handle;

		/**
		 * @brief 0 for blocking, non 0 for non blocking
		 */
		u_long blockingMode;

		/**
		 * @brief 0 for blocking, non 0 for non blocking
		 */
		u_long listenSocketBlockingMode;

	protected:
		void createListenSocket();

		virtual void receiveConnections(const std::function<void()>& onStartServer);

		virtual void serve(std::string ip, SOCKET clientSocket, sockaddr address);

		/**
		 * @brief Serving each client connection
		 * @param ip Client IP address
		 * @param clientSocket Client socket
		 * @param address Structure used to store most addresses.
		 * @param cleanup Move this function if you want made cleanup by yourself
		 */
		virtual void clientConnection(const std::string& ip, SOCKET clientSocket, sockaddr address, std::function<void()>& cleanup) = 0;

		virtual void onConnectionReceive(SOCKET clientSocket, sockaddr address);

		virtual void onInvalidConnectionReceive();
		
	protected:
		template<typename DataT>
		static int sendBytes(SOCKET clientSocket, const DataT* const data, int size);

		template<typename DataT>
		static int receiveBytes(SOCKET clientSocket, DataT* const data, int size);

	public:
		/**
		 * @brief Get client IP address
		 * @param address 
		 * @return 
		 */
		static std::string getClientIpV4(sockaddr address);

		/**
		 * @brief Get server IP address
		 * @return 
		 */
		std::string getServerIpV4() const;

		/**
		 * @brief Get client port
		 * @param address 
		 * @return 
		 */
		static uint16_t getClientPortV4(sockaddr address);

		/**
		 * @brief Get server port
		 * @return 
		 */
		uint16_t getServerPortV4() const;

		/**
		 * @brief Get BaseTCPServer version
		 * @return 
		 */
		static std::string getVersion();

	public:
		/// @brief 
		/// @param port Server's port
		/// @param ip Server's ip
		/// @param timeout recv function timeout in milliseconds, 0 wait for upcoming data
		/// @param multiThreading Each client in separate thread
		/// @param listenSocketBlockingMode Blocking mode for listen socket (0 - blocking, non 0 - non blocking)
		/// @param freeDLL Unload Ws2_32.dll in destructor(Windows only parameter)
		BaseTCPServer(std::string_view port, std::string_view ip = "0.0.0.0", DWORD timeout = 0, bool multiThreading = true, u_long listenSocketBlockingMode = 0, bool freeDLL = true);

		/**
		 * @brief Start server in separate thread
		 * @param wait Wait server serving in current thread
		 * @param onStartServer Call function before accept first connection
		 */
		virtual void start(bool wait = false, const std::function<void()>& onStartServer = []() {});

		/**
		 * @brief Stop receiving new connections
		 * @param wait Wait all clients tasks
		 */
		virtual void stop(bool wait = true);

		/**
		 * @brief Kick specific client
		 * @param ip 
		 */
		virtual void kick(const std::string& ip);

		/**
		 * @brief Kick all clients
		 */
		virtual void kickAll();

		/**
		 * @brief Is server accept new connections
		 * @return 
		 */
		bool isServerRunning() const;

		/**
		 * @brief Is server's listen socket in blocking mode
		 * @return 
		 */
		bool isListenSocketInBlockingMode() const;

		/**
		 * @brief Is client's socket in blocking mode
		 * @return 
		 */
		bool isAcceptedSocketsInBlockingMode() const;

		/**
		 * @brief Set blocking mode for all connected sockets
		 * @param block 
		 */
		void setAcceptedSocketsBlockingMode(bool block);

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

		/**
		 * @brief Get all clients ip - sockets
		 * @return 
		 */
		std::vector<std::pair<std::string, std::vector<SOCKET>>> getClients() const;

		virtual ~BaseTCPServer();
	};

	template<typename DataT>
	int BaseTCPServer::sendBytes(SOCKET clientSocket, const DataT* const data, int size)
	{
		int lastSend = 0;
		int totalSent = 0;

		do
		{
			lastSend = send(clientSocket, reinterpret_cast<const char*>(data) + totalSent, size - totalSent, NULL);

			if (lastSend == SOCKET_ERROR)
			{
				THROW_WEB_EXCEPTION;
			}
			else if (!lastSend)
			{
				return totalSent;
			}

			totalSent += lastSend;

		} while (totalSent < size);

		return totalSent;
	}

	template<typename DataT>
	int BaseTCPServer::receiveBytes(SOCKET clientSocket, DataT* const data, int size)
	{
		int lastReceive = recv(clientSocket, reinterpret_cast<char*>(data), size, NULL);

		if (lastReceive == SOCKET_ERROR)
		{
			THROW_WEB_EXCEPTION;
		}

		return lastReceive;
	}
}
