#pragma once
#include <array>
#include "../Vector/Vector3.hpp"

namespace CU
{
	template<typename T>
	class Matrix4x4;

	template<typename T>
	class Matrix3x3
	{
	public:
		Matrix3x3<T>();
		Matrix3x3<T>(const Matrix3x3<T>& aMatrix);
		Matrix3x3<T>(const Matrix4x4<T>& aMatrix);
		Matrix3x3<T>(std::initializer_list<T> aList);
		~Matrix3x3() = default;

		T& operator()(const int aRow, const int aColumn);
		const T& operator()(const int aRow, const int aColumn) const;

		T& operator[](const int aIndex);
		const T& operator[](const int aIndex) const;

		Matrix3x3<T>& operator=(const Matrix3x3<T>& aMatrix);

		Matrix3x3<T> GetTranspose() const;
		void Transpose();

		static Matrix3x3<T> CreateRotationAroundX(const T aAngleInRadians);
		static Matrix3x3<T> CreateRotationAroundY(const T aAngleInRadians);
		static Matrix3x3<T> CreateRotationAroundZ(const T aAngleInRadians);

		static Matrix3x3<T> GetTranspose(const Matrix3x3<T>& aMatrixToTranspose);

		static const Matrix3x3<T> Zero;

	private:
		std::array<T, 9> myData;
	};

	template <class T> 
	Matrix3x3<T> operator+(const Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		return
		{
			aMatrix0[0] + aMatrix1[0],
			aMatrix0[1] + aMatrix1[1],
			aMatrix0[2] + aMatrix1[2],

			aMatrix0[3] + aMatrix1[3],
			aMatrix0[4] + aMatrix1[4],
			aMatrix0[5] + aMatrix1[5],

			aMatrix0[6] + aMatrix1[6],
			aMatrix0[7] + aMatrix1[7],
			aMatrix0[8] + aMatrix1[8]
		};
	}

	template <class T> 
	Matrix3x3<T> operator-(const Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		return
		{
			aMatrix0[0] - aMatrix1[0],
			aMatrix0[1] - aMatrix1[1],
			aMatrix0[2] - aMatrix1[2],

			aMatrix0[3] - aMatrix1[3],
			aMatrix0[4] - aMatrix1[4],
			aMatrix0[5] - aMatrix1[5],

			aMatrix0[6] - aMatrix1[6],
			aMatrix0[7] - aMatrix1[7],
			aMatrix0[8] - aMatrix1[8]
		};
	}

	template <class T> 
	Matrix3x3<T> operator*(const Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		Matrix3x3<T> result;

		result[0] = 
			aMatrix0[0] * aMatrix1[0] +
			aMatrix0[1] * aMatrix1[3] +
			aMatrix0[2] * aMatrix1[6];

		result[1] =
			aMatrix0[0] * aMatrix1[1] +
			aMatrix0[1] * aMatrix1[4] +
			aMatrix0[2] * aMatrix1[7];

		result[2] =
			aMatrix0[0] * aMatrix1[2] +
			aMatrix0[1] * aMatrix1[5] +
			aMatrix0[2] * aMatrix1[8];


		result[3] =
			aMatrix0[3] * aMatrix1[0] +
			aMatrix0[4] * aMatrix1[3] +
			aMatrix0[5] * aMatrix1[6];

		result[4] =
			aMatrix0[3] * aMatrix1[1] +
			aMatrix0[4] * aMatrix1[4] +
			aMatrix0[5] * aMatrix1[7];

		result[5] =
			aMatrix0[3] * aMatrix1[2] +
			aMatrix0[4] * aMatrix1[5] +
			aMatrix0[5] * aMatrix1[8];


		result[6] =
			aMatrix0[6] * aMatrix1[0] +
			aMatrix0[7] * aMatrix1[3] +
			aMatrix0[8] * aMatrix1[6];

		result[7] =
			aMatrix0[6] * aMatrix1[1] +
			aMatrix0[7] * aMatrix1[4] +
			aMatrix0[8] * aMatrix1[7];

		result[8] =
			aMatrix0[6] * aMatrix1[2] +
			aMatrix0[7] * aMatrix1[5] +
			aMatrix0[8] * aMatrix1[8];

		return result;
	}

	template <class T> 
	Vector3<T> operator*(const Matrix3x3<T>& aMatrix, const Vector3<T>& aVector)
	{
		return
		{
			aVector.x * aMatrix[0] + aVector.y * aMatrix[3] + aVector.z * aMatrix[6],
			aVector.x * aMatrix[1] + aVector.y * aMatrix[4] + aVector.z * aMatrix[7],
			aVector.x * aMatrix[2] + aVector.y * aMatrix[5] + aVector.z * aMatrix[8]
		};
	}

	template <class T>
	Matrix3x3<T> operator*(const Matrix3x3<T>& aMatrix, const float aScalar)
	{
		return
		{
			aMatrix[0] * aScalar,
			aMatrix[1] * aScalar,
			aMatrix[2] * aScalar,

			aMatrix[3] * aScalar,
			aMatrix[4] * aScalar,
			aMatrix[5] * aScalar,

			aMatrix[6] * aScalar,
			aMatrix[7] * aScalar,
			aMatrix[8] * aScalar
		};
	}

	template <class T> 
	void operator+=(Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		aMatrix0 = aMatrix0 + aMatrix1;
	}

	template <class T> 
	void operator-=(Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		aMatrix0 = aMatrix0 - aMatrix1;
	}

	template <class T> 
	void operator*=(Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		aMatrix0 = aMatrix0 * aMatrix1;
	}

	template <class T> 
	void operator*=(const Matrix3x3<T>& aMatrix, Vector3<T>& aVector)
	{
		aVector = aMatrix * aVector;
	}

	template <class T>
	void operator*=(Matrix3x3<T>& aMatrix, const float aScalar)
	{
		aMatrix = aMatrix * aScalar;
	}


	template<typename T>
	inline Matrix3x3<T>::Matrix3x3()
	{
		myData =
		{
			T(1), T(0), T(0),
			T(0), T(1), T(0),
			T(0), T(0), T(1)
		};
	}

	template<typename T>
	inline Matrix3x3<T>::Matrix3x3(const Matrix3x3<T>& aMatrix)
	{
		memcpy_s(&myData.front(), sizeof(T) * 9, &aMatrix.myData.front(), sizeof(T) * 9);
	}

	template<typename T>
	inline Matrix3x3<T>::Matrix3x3(const Matrix4x4<T>& aMatrix)
	{
		const rsize_t rowLength = sizeof(T) * 3;
		memcpy_s(&myData[0], rowLength, &aMatrix[0], rowLength);
		memcpy_s(&myData[3], rowLength, &aMatrix[4], rowLength);
		memcpy_s(&myData[6], rowLength, &aMatrix[8], rowLength);
	}

	template<typename T>
	inline Matrix3x3<T>::Matrix3x3(std::initializer_list<T> aList)
	{
		memcpy_s(&myData.front(), sizeof(T) * 9, aList.begin(), sizeof(T) * 9);
	}

	template<typename T>
	inline T& Matrix3x3<T>::operator()(const int aRow, const int aColumn)
	{
		const int index = (aRow - 1) * 3 + (aColumn - 1);
		return myData[index];
	}

	template<typename T>
	inline const T& Matrix3x3<T>::operator()(const int aRow, const int aColumn) const
	{
		const int index = (aRow - 1) * 3 + (aColumn - 1);
		return myData[index];
	}

	template<typename T>
	inline T& Matrix3x3<T>::operator[](const int aIndex)
	{
		return myData[aIndex];
	}

	template<typename T>
	inline const T& Matrix3x3<T>::operator[](const int aIndex) const
	{
		return myData[aIndex];
	}

	template<typename T>
	inline Matrix3x3<T>& Matrix3x3<T>::operator=(const Matrix3x3<T>& aMatrix)
	{
		memcpy_s(&myData.front(), sizeof(T) * 9, &aMatrix.myData.front(), sizeof(T) * 9);
		return *this;
	}

	template<typename T>
	inline Matrix3x3<T> Matrix3x3<T>::GetTranspose() const
	{
		return GetTranspose(*this);
	}

	template<typename T>
	inline void Matrix3x3<T>::Transpose()
	{
		myData =
		{
			myData[0], myData[3], myData[6],
			myData[1], myData[4], myData[7],
			myData[2], myData[5], myData[8]
		};
	}


	template<typename T>
	inline Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundX(const T aAngleInRadians)
	{
		const T c = std::cos(aAngleInRadians);
		const T s = std::sin(aAngleInRadians);

		return
		{
			T(1), T(0), T(0),
			T(0), c, s,
			T(0), -s, c
		};
	}

	template<typename T>
	inline Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundY(const T aAngleInRadians)
	{
		const T c = std::cos(aAngleInRadians);
		const T s = std::sin(aAngleInRadians);

		return
		{
			c, T(0), -s,
			T(0), T(1), T(0),
			s, T(0), c
		};
	}

	template<typename T>
	inline Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundZ(const T aAngleInRadians)
	{
		const T c = std::cos(aAngleInRadians);
		const T s = std::sin(aAngleInRadians);

		return
		{
			c, s, T(0),
			-s, c, T(0),
			T(0), T(0), T(1)
		};
	}

	template<typename T>
	inline Matrix3x3<T> Matrix3x3<T>::GetTranspose(const Matrix3x3<T>& aMatrix)
	{
		return
		{
			aMatrix[0], aMatrix[3], aMatrix[6],
			aMatrix[1], aMatrix[4], aMatrix[7],
			aMatrix[2], aMatrix[5], aMatrix[8]
		};
	}


	template<typename T>
	const Matrix3x3<T> Matrix3x3<T>::Zero({ 0, 0, 0, 0, 0, 0, 0, 0, 0 });

	typedef Matrix3x3<float> Matrix3x3f;
}