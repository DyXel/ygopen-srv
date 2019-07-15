#ifndef __DUEL_OBSERVER_HPP__
#define __DUEL_OBSERVER_HPP__
#include "util/buffer_manipulator.hpp"

namespace YGOpen
{

class DuelObserver
{
public:
	virtual void OnNotify(void* buffer, size_t length) = 0;
};

} // namespace YGOpen

#endif // __DUEL_OBSERVER_HPP__
