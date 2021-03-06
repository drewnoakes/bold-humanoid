#pragma once

#include "Point.h"

namespace bold
{
	class Vector3D
	{
	private:

	protected:

	public:
		double X;
		double Y;
		double Z;

		Vector3D();
		Vector3D(double x, double y, double z);
		Vector3D(const Point3D &pt1, const Point3D &pt2);
		Vector3D(const Vector3D &vector);
		~Vector3D();

		double Length();
		void Normalize();
		double Dot(const Vector3D &vector);
		Vector3D Cross(const Vector3D &vector);
		double AngleBetween(Vector3D &vector);
		double AngleBetween(Vector3D &vector, Vector3D &axis);

		Vector3D & operator = (const Vector3D &vector);
		Vector3D & operator += (const Vector3D &vector);
		Vector3D & operator -= (const Vector3D &vector);
		Vector3D & operator += (const double value);
		Vector3D & operator -= (const double value);
		Vector3D & operator *= (const double value);
		Vector3D & operator /= (const double value);
		Vector3D operator + (const Vector3D &vector);
		Vector3D operator - (const Vector3D &vector);
		Vector3D operator + (const double value);
		Vector3D operator - (const double value);
		Vector3D operator * (const double value);
		Vector3D operator / (const double value);
	};
}
