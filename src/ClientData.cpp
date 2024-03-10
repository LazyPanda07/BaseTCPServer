#include "ClientData.h"

#include <algorithm>

using namespace std;

namespace web
{
	void ClientData::add(const string& ip, SOCKET socket)
	{
		unique_lock<mutex> lock(dataMutex);

		data[ip].push_back(socket);
	}

	void ClientData::remove(const string& ip, SOCKET socket)
	{
		unique_lock<mutex> lock(dataMutex);
		
		erase(data[ip], socket);
	}

	size_t ClientData::getNumberOfClients() const
	{
		unique_lock<mutex> lock(dataMutex);

		return data.size();
	}

	size_t ClientData::getNumberOfConnections() const
	{
		size_t result = 0;

		unique_lock<mutex> lock(dataMutex);

		for (const auto& [key, value] : data)
		{
			result += value.size();
		}

		return result;
	}
}
