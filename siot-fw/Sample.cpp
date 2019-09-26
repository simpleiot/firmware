
#include "Sample.h"


String Sample::string()
{
	return String::format("%s:%f (%s)", id.c_str(), value, type.c_str());
}
