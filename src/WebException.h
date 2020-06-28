#pragma once

#ifndef __WEB_EXCEPTION__
#define __WEB_EXCEPTION__

namespace web
{
	class WebException
	{
	private:
		char* data;

	public:
		WebException();

		WebException(const WebException& other) = default;

		WebException(WebException&& other) noexcept;

		const char* what() const noexcept;

		~WebException();
	};
}

#endif // !__WEB_EXCEPTION__