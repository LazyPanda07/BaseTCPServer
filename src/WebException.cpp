#include "WebException.h"

#include <WinSock2.h>
#include <winbase.h>

namespace web
{
	WebException::WebException()
	{
		data = std::to_string(WSAGetLastError());
	}

	WebException::WebException(WebException&& other) noexcept
	{
		data = std::move(other.data);
	}

	const char* WebException::what() const noexcept
	{
		return data.data();
	}
}