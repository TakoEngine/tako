#include "Math.hpp"
#include "Utility.hpp"

namespace tako
{
    const Matrix4 Matrix4::identity =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

	void Matrix4::Print()
	{
		LOG("{} {} {} {}\n{} {} {} {}\n{} {} {} {}\n{} {} {} {}",
			m[0], m[1], m[2], m[3],
			m[4], m[5], m[6], m[7],
			m[8], m[9], m[10], m[11],
			m[12], m[13], m[14], m[15]);
	}
}
