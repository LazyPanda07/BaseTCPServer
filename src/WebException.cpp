#include "WebException.h"

#include <WinSock2.h>
#include <winbase.h>

namespace web
{
	WebException::WebException() : data(nullptr)
	{
		FormatMessageA
		(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			WSAGetLastError(),
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			reinterpret_cast<char*>(&data),
			NULL,
			nullptr
		);
	}

	WebException::WebException(WebException&& other) noexcept
	{
		data = other.data;
		other.data = nullptr;
	}

	const char* WebException::what() const noexcept
	{
		return data;
	}

	WebException::~WebException()
	{
		LocalFree(data);
	}
}