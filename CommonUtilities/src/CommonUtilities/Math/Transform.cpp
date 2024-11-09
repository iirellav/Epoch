#include "Transform.h"

namespace CU
{
	Transform::Transform(Vector3f aTranslation, Vector3f aRotation, Vector3f aScale) : myTranslation(aTranslation), myRotation(aRotation), myRotationQuat(aRotation), myScale(aScale), myIsDirty(true) {}

	Transform::Transform(const Matrix4x4f& aMatrix) : myMatrix(aMatrix)
	{
		aMatrix.Decompose(myTranslation, myRotation, myScale);
		myRotationQuat = Quatf(myRotation);
		myIsDirty = false;
	}

	const Matrix4x4f& Transform::GetMatrix()
	{
		if (myIsDirty)
		{
			const Matrix4x4f scaleMatrix = Matrix4x4f::CreateScaleMatrix(myScale);
			const Matrix4x4f rotationMatrix = myRotationQuat.GetRotationMatrix4x4f();
			const Matrix4x4f translationMatrix = Matrix4x4f::CreateTranslationMatrix(myTranslation);

			myMatrix = scaleMatrix * rotationMatrix * translationMatrix;
			myIsDirty = false;
		}

		return myMatrix;
	}

	void Transform::SetTransform(const Matrix4x4f& aMatrix)
	{
		aMatrix.Decompose(myTranslation, myRotation, myScale);
		myRotationQuat = Quatf(myRotation);
		myMatrix = aMatrix;
		myIsDirty = false;
	}

	void Transform::SetTransform(const Transform& aTransform)
	{
		myTranslation = aTransform.myTranslation;
		myRotation = aTransform.myRotation;
		myScale = aTransform.myScale;
		myRotationQuat = aTransform.myRotationQuat;
		myMatrix = aTransform.myMatrix;
		myIsDirty = false;
	}

	void Transform::SetTranslation(const Vector3f& aTranslation)
	{
		myTranslation = aTranslation;
		myIsDirty = true;
	}

	void Transform::SetTranslation(float aX, float aY, float aZ)
	{
		myTranslation.x = aX;
		myTranslation.y = aY;
		myTranslation.z = aZ;
		myIsDirty = true;
	}

	void Transform::SetRotation(const Vector3f& aRotation)
	{
		myRotation = aRotation;
		myRotationQuat = Quatf(myRotation);
		myIsDirty = true;
	}

	void Transform::SetRotation(float aX, float aY, float aZ)
	{
		myRotation = Vector3f(aX, aY, aZ);
		myRotationQuat = Quatf(myRotation);
		myIsDirty = true;
	}

	void Transform::SetScale(const Vector3f& aScale)
	{
		myScale = aScale;
		myIsDirty = true;
	}

	void Transform::SetScale(float aX, float aY, float aZ)
	{
		myScale.x = aX;
		myScale.y = aY;
		myScale.z = aZ;
		myIsDirty = true;
	}

	void Transform::Translate(const Vector3f& aTranslation)
	{
		myTranslation += aTranslation;
		myIsDirty = true;
	}

	void Transform::Rotate(const Vector3f& aRotation)
	{
		myRotation += aRotation;
		myRotationQuat = Quatf(myRotation);
		myIsDirty = true;
	}

	void Transform::Scale(const Vector3f& aScale)
	{
		myScale += aScale;
		myIsDirty = true;
	}

	void Transform::RotateAround(const Vector3f& aPoint, const Vector3f& aAxis, float aAngle)
	{
		const Quatf quat = Quatf(aAxis, aAngle);
		myRotationQuat *= quat;
		myRotation += quat.GetEulerAngles();

		myTranslation = Quatf::RotateVectorByQuaternion((myTranslation - aPoint), quat) + aPoint;
		myIsDirty = true;
	}

	void Transform::LookAt(const Vector3f& aTarget, const Vector3f& aUp)
	{
		const Vector3f forwardDir = (aTarget - myTranslation).GetNormalized();

		if (forwardDir == aUp) return;

		const Vector3f rightDir = aUp.Cross(forwardDir).GetNormalized();
		const Vector3f upDir = forwardDir.Cross(rightDir).GetNormalized();

		const Matrix3x3f rotationMatrix = Matrix4x4f::CreateRotationMatrix(rightDir, upDir, forwardDir);
		myRotationQuat = Quatf(rotationMatrix);
		myRotation = myRotationQuat.GetEulerAngles();
		myIsDirty = true;
	}
}
