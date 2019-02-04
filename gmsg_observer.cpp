#include "gmsg_observer.hpp"
#include "../ocgcore-proto/io_gmsg_stream.hpp"

namespace YGOpen
{

struct GMsgObserver::impl
{
	IGMsgEncoder encoder;
};

GMsgObserver::GMsgObserver() : pimpl(new impl())
{}

GMsgObserver::~GMsgObserver() = default;

void GMsgObserver::OnNotify(void* buffer, size_t length)
{
	Core::GMsg gmsg = pimpl->encoder.Encode(buffer, length);
}

} // namespace YGOpen
