#include "gmsg_observer.hpp"

#include <string>
#include <iostream>
#include <google/protobuf/text_format.h>

namespace YGOpen
{

GMsgObserver::GMsgObserver()
{
	
}

void GMsgObserver::OnNotify(void* buffer, size_t length)
{
	Core::GMsg gmsg = encoder.Encode(buffer, length);
	std::string str;
	google::protobuf::TextFormat::PrintToString(gmsg, &str);
	std::cout << str << std::endl;
}

} // namespace YGOpen
