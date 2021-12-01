#ifndef JSON_EXCEPTION_H
#define JSON_EXCEPTION_H

#include <string>
using namespace std;

namespace Json {
	class JsonException : public exception
	{
	public:
		JsonException(string m="exception!") : msg(m) {}
		~JsonException() throw() {}
		const char* what() const throw() { return msg.c_str(); }

	private:
		const string msg;
	};
}
#endif