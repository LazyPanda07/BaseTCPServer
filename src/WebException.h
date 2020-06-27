#pragma once

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