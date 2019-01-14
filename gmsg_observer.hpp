#ifndef __GMSG_OBSERVER_HPP__
#define __GMSG_OBSERVER_HPP__
#include "duel_observer.hpp"
#include "../ocgcore-proto/io_gmsg_stream.hpp"

namespace YGOpen
{

class GMsgObserver : public DuelObserver
{
	IGMsgEncoder encoder;
public:
	GMsgObserver();

	virtual void OnNotify(void* buffer, size_t length);
};

} // namespace YGOpen

#endif // __GMSG_OBSERVER_HPP__
