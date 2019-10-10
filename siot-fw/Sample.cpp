
#include "Sample.h"


String Sample::string()
{
	return String::format("%s:%f (%s)", id.c_str(), value, type.c_str());
}


void Sample::toJSON(JsonWriter *jw)
{
	jw->insertKeyValue("id", id);
	jw->insertKeyValue("type", type);
	jw->insertKeyValue("value", value);
}
