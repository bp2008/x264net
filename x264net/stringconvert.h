#pragma once
#pragma managed( push, off )
#include <string>
#pragma managed( pop )
System::String^ getSystemString(std::string const& source);
std::string getStdString(System::String^ source);
