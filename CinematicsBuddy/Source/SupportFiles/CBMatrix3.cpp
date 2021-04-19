#include "CBMatrix3.h"

CBUtils::Matrix3::Matrix3()
	: Forward(Vector{1,0,0}), Right(Vector{0,1,0}), Up(Vector{0,0,1}) {}

CBUtils::Matrix3::Matrix3(Vector f, Vector r, Vector u)
	: Forward(f), Right(r), Up(u) {}

CBUtils::Matrix3::Matrix3(Quat InQuat)
{
    Forward = RotateVectorWithQuat(Vector(1, 0, 0), InQuat);
	Right   = RotateVectorWithQuat(Vector(0, 1, 0), InQuat);
	Up      = RotateVectorWithQuat(Vector(0, 0, 1), InQuat);
	Normalize();
}

CBUtils::Matrix3::Matrix3(Rotator InRotator)
{
    Quat q = RotatorToQuat(InRotator);
	*this = Matrix3(q);
}

Quat CBUtils::Matrix3::ToQuat() const
{
	//https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/

	Quat q;

	float trace = Forward.X + Right.Y + Up.Z;
	if( trace > 0 )
	{
		float s = 0.5f / sqrtf(trace+ 1.0f);
		q.W =   0.25f / s;
		q.X = ( Right.Z   - Up.Y      ) * s;
		q.Y = ( Up.X      - Forward.Z ) * s;
		q.Z = ( Forward.Y - Right.X   ) * s;
	}
	else
	{
		if ( Forward.X > Right.Y && Forward.X > Up.Z )
		{
			float s = 2.0f * sqrtf( 1.0f + Forward.X - Right.Y - Up.Z);
			q.W = ( Right.Z - Up.Y      ) / s;
			q.X =   0.25f * s;
			q.Y = ( Right.X + Forward.Y ) / s;
			q.Z = ( Up.X    + Forward.Z ) / s;
		}
		else if (Right.Y > Up.Z)
		{
			float s = 2.0f * sqrtf( 1.0f + Right.Y - Forward.X - Up.Z);
			q.W = ( Up.X    - Forward.Z ) / s;
			q.X = ( Right.X + Forward.Y ) / s;
			q.Y =   0.25f * s;
			q.Z = ( Up.Y    + Right.Z   ) / s;
		}
		else
		{
			float s = 2.0f * sqrtf( 1.0f + Up.Z - Forward.X - Right.Y );
			q.W = ( Forward.Y - Right.X   ) / s;
			q.X = ( Up.X      + Forward.Z ) / s;
			q.Y = ( Up.Y      + Right.Z   ) / s;
			q.Z =   0.25f * s;
		}
	}

	return q;
}

Rotator CBUtils::Matrix3::ToRotator() const
{
	Quat q = ToQuat();
	return QuatToRotator(q);
}

CBUtils::Matrix3 CBUtils::Matrix3::RotateWithQuat(Quat InQuat, bool bShouldNormalize)
{
	Forward = RotateVectorWithQuat(Forward, InQuat);
	Right = RotateVectorWithQuat(Right, InQuat);
	Up = RotateVectorWithQuat(Up, InQuat);

	if(bShouldNormalize)
	{
		Normalize();
	}

	return *this;
}

void CBUtils::Matrix3::Normalize()
{
	Forward.normalize();
	Right.normalize();
	Up.normalize();
}

const CBUtils::Matrix3 CBUtils::Matrix3::Identity()
{
	return Matrix3();
}
