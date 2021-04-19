#pragma once
#include "bakkesmod/wrappers/wrapperstructs.h"

// Matrix3 class extracted from RenderingTools and trimmed for easier including
// https://github.com/CinderBlocc/RenderingTools/blob/master/Objects/Matrix3.h

namespace CBUtils
{
    class Matrix3
	{
	public:
		Vector Forward;
		Vector Right;
		Vector Up;

		// CONSTRUCTORS
		explicit Matrix3();
		explicit Matrix3(Vector InForward, Vector InRight, Vector InUp);
		explicit Matrix3(Quat InQuat);
		explicit Matrix3(Rotator InRotator);

		// FUNCTIONS		
		Quat ToQuat() const;
		Rotator ToRotator() const;

		Matrix3 RotateWithQuat(Quat InQuat, bool bShouldNormalize = false);

		void Normalize();
		const static Matrix3 Identity();
	};
}
