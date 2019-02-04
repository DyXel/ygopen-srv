#ifndef __GMSG_OBSERVER_HPP__
#define __GMSG_OBSERVER_HPP__
#include <memory>
#include "duel_observer.hpp"

namespace YGOpen
{

class MsgCodecObserver : public DuelObserver
{
	struct impl;
	std::unique_ptr<impl> pimpl;

public:
	MsgCodecObserver();
	~MsgCodecObserver();
	virtual void OnNotify(void* buffer, size_t length);
};

} // namespace YGOpen

#endif // __GMSG_OBSERVER_HPP__
