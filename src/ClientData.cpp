#include "ClientData.h"

#include <algorithm>

using namespace std;

namespace web
{
	void ClientData::insert(string&& ip, SOCKET clientSocket, future<void>&& servingFunction) noexcept
	{
		unique_lock<mutex> lock(readWriteLock);

		data.emplace(move(ip), make_pair(clientSocket, move(servingFunction)));
	}

	vector<pair<SOCKET, future<void>>> ClientData::operator [] (const string& ip)
	{
		vector<pair<SOCKET, future<void>>> result;

		{
			unique_lock<mutex> lock(readWriteLock);
			auto it = data.equal_range(ip);

			result.reserve(distance(it.first, it.second));

			while (true)
			{
				auto node = data.extract(ip);

				if (!node)
				{
					break;
				}

				result.emplace_back(move(node.mapped()));
			}
		}

		return result;
	}

	void ClientData::erase(const string& ip) noexcept
	{
		unique_lock<mutex> lock(readWriteLock);

		if (data.find(ip) != end(data))
		{
			data.erase(ip);
		}
	}

	vector<pair<string, SOCKET>> ClientData::getClients() noexcept
	{
		vector<pair<string, SOCKET>> result;
		unique_lock<mutex> lock(readWriteLock);

		result.reserve(data.size());

		for (const auto& [ip, clientData] : data)
		{
			result.emplace_back(ip, clientData.first);
		}

		return result;
	}
}
