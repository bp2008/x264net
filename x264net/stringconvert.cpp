#include "stringconvert.h"
#include "clix.h"
using namespace clix;
System::String^ getSystemString(std::string const& source)
{
	if (source.length() < 1)
		return "";
	return marshalString<E_UTF8>(source);
}
std::string getStdString(System::String^ source)
{
	if (System::String::IsNullOrEmpty(source))
		return "";
	return marshalString<E_UTF8>(source);
}