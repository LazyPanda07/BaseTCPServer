#include "WebException.h"

#include <WinSock2.h>
#include <winbase.h>

namespace web
{
	WebException::WebException() :
		runtime_error("")
	{
		switch (WSAGetLastError())
		{
		case WSABASEERR:
			data = "No Error";

			break;

		case WSAEINTR:
			data = "Interrupted system call";

			break;

		case WSAEBADF:
			data = "Bad file number";

			break;

		case WSAEACCES:
			data = "Permission denied";

			break;

		case WSAEFAULT:
			data = "Bad address";

			break;

		case WSAEINVAL:
			data = "Invalid argument";

			break;

		case WSAEMFILE:
			data = "Too many open files";

			break;

		case WSAEWOULDBLOCK:
			data = "Operation would block";

			break;

		case WSAEINPROGRESS:
			data = "Operation now in progress";

			break;

		case WSAEALREADY:
			data = "Operation already in progress";

			break;

		case WSAENOTSOCK:
			data = "Socket operation on non-socket";

			break;

		case WSAEDESTADDRREQ:
			data = "Destination address required";

			break;

		case WSAEMSGSIZE:
			data = "Message too long";

			break;

		case WSAEPROTOTYPE:
			data = "Protocol wrong type for socket";

			break;

		case WSAENOPROTOOPT:
			data = "Bad protocol option";

			break;

		case WSAEPROTONOSUPPORT:
			data = "Protocol not supported";

			break;

		case WSAESOCKTNOSUPPORT:
			data = "Socket type not supported";

			break;

		case WSAEOPNOTSUPP:
			data = "Operation not supported on socket";

			break;

		case WSAEPFNOSUPPORT:
			data = "Protocol family not supported";

			break;

		case WSAEAFNOSUPPORT:
			data = "Address family not supported by protocol family";

			break;

		case WSAEADDRINUSE:
			data = "Address already in use";

			break;

		case WSAEADDRNOTAVAIL:
			data = "Can't assign requested address";

			break;

		case WSAENETDOWN:
			data = "Network is down";

			break;

		case WSAENETUNREACH:
			data = "Network in unreachable";

			break;

		case WSAENETRESET:
			data = "Net dropped connection or reset";

			break;

		case WSAECONNABORTED:
			data = "Software caused connection abort";

			break;

		case WSAECONNRESET:
			data = "Connection reste by peer";

			break;

		case WSAENOBUFS:
			data = "No buffer space available";

			break;

		case WSAEISCONN:
			data = "Socket is already connected";

			break;

		case WSAENOTCONN:
			data = "Socket is not connected";

			break;

		case WSAESHUTDOWN:
			data = "Can't send after socket shutdown";

			break;

		case WSAETOOMANYREFS:
			data = "Too many references, can't splice";

			break;

		case WSAETIMEDOUT:
			data = "Connection timed out";

			break;

		case WSAECONNREFUSED:
			data = "Connection refused";

			break;

		case WSAELOOP:
			data = "too many levels of symbolic links";

			break;

		case WSAENAMETOOLONG:
			data = "File name too long";

			break;

		case WSAEHOSTDOWN:
			data = "Host is down";

			break;

		case WSAEHOSTUNREACH:
			data = "No Route to Host";

			break;

		case WSAENOTEMPTY:
			data = "Directory not empty";

			break;

		case WSAEPROCLIM:
			data = "Too many processes";

			break;

		case WSAEUSERS:
			data = "too many users";

			break;

		case WSAEDQUOT:
			data = "Disc Quota Exceeded";
			
			break;

		case WSAESTALE:
			data = "Stale NFS file handle";

			break;

		case WSASYSNOTREADY:
			data = "Network SubSystem is unavailable";

			break;

		case WSAVERNOTSUPPORTED:
			data = "WINSOCK DLL Version out of range";

			break;

		case WSANOTINITIALISED:
			data = "Successful WSASTARTUP not yet performed";

			break;

		case WSAEREMOTE:
			data = "Too many level of remote in path";

			break;

		case WSAHOST_NOT_FOUND:
			data = "Host not found";
			
			break;

		case WSATRY_AGAIN:
			data = "Non-Authoritative Host not found";

			break;

		case WSANO_RECOVERY:
			data = "Non-Recoverable errors: FORMERR, REFUSED, NOTIMP";

			break;

		case WSANO_DATA:
			data = "Valid name, no data record of requested type";

			break;

		default:
			data = std::to_string(WSAGetLastError());

			break;
		}
	}

	WebException::WebException(WebException&& other) noexcept :
		runtime_error(""),
		data(std::move(other.data))
	{
		
	}

	const char* WebException::what() const noexcept
	{
		return data.data();
	}
}
