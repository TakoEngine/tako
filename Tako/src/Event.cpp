#include "Event.hpp"

namespace tako
{

	std::ostream& operator<<(std::ostream& os, const Event& evt)
	{
		evt.DebugPrint(os);
		return os;
	}
}

