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

	Matrix4 Matrix4::cameraViewMatrix(const Vector3 position, const Quaternion& rotation)
	{
		auto mat = rotation.ToRotationMatrix();
		mat[12] = -position.x;
		mat[13] = -position.y;
		mat[14] = -position.z;

		return Matrix4::inverse(mat);
	}

	Matrix4 Matrix4::DirectionToRotation(const Vector3& direction, const Vector3& up)
	{
		//https://stackoverflow.com/a/18574797
		Vector3 xAxis = Vector3::cross(up, direction);
		xAxis.normalize();

		Vector3 yAxis = Vector3::cross(direction, xAxis);
		yAxis.normalize();

		auto m = Matrix4::identity;
		m[0] = xAxis.x;
		m[1] = yAxis.x;
		m[2] = direction.x;

		m[4] = xAxis.y;
		m[5] = yAxis.y;
		m[6] = direction.y;

		m[8] = xAxis.z;
		m[9] = yAxis.z;
		m[10] = direction.z;

		return m;
	}


}
