#include "gmsg_observer.hpp"
#include "../ocgcore-proto/msg_codec.hpp"

namespace YGOpen
{

struct MsgCodecObserver::impl
{
	MsgEncoder encoder;
};

MsgCodecObserver::MsgCodecObserver() : pimpl(new impl())
{}

MsgCodecObserver::~MsgCodecObserver() = default;

void MsgCodecObserver::OnNotify(void* buffer, size_t length)
{
	Core::GMsg gmsg = pimpl->encoder.Encode(buffer, length);
}

} // namespace YGOpen
