#pragma once

#include "stdafx.h"

#include <vector>
#include <map>
#include <limits>

#undef max

using namespace std;

#ifdef _LINUX
#include <wx/wx.h>

typedef int BOOL;
typedef wxPoint CPoint;
typedef wxRect CRect;
typedef unsigned int UINT;
#endif // _LINUX


// ----------------------------------------------------------------------------
/*
// X, Y, Z, Nx, Ny, Nz, Ambient, Diffuse, Emissive, Specular
// (Normal vectors - bit 5, Diffuse, Emissive, Specular - bit 25, 26 & 27)
*/
#define VERTEX_LENGTH  10

// ------------------------------------------------------------------------------------------------
// X, Y, Z, Nx, Ny, Nz
#define GEOMETRY_VBO_VERTEX_LENGTH  6

// ------------------------------------------------------------------------------------------------
#define DEFAULT_CIRCLE_SEGMENTS 36

static  const   int64_t    flagbit0 = 1;                           // 2^^0                          0000.0000..0000.0001
static  const   int64_t    flagbit1 = 2;                           // 2^^1                          0000.0000..0000.0010
static  const   int64_t    flagbit2 = 4;                           // 2^^2                          0000.0000..0000.0100
static  const   int64_t    flagbit3 = 8;                           // 2^^3                          0000.0000..0000.1000

static  const   int64_t    flagbit4 = 16;                          // 2^^4                          0000.0000..0001.0000
static  const   int64_t    flagbit5 = 32;                          // 2^^5                          0000.0000..0010.0000
static  const   int64_t    flagbit6 = 64;                          // 2^^6                          0000.0000..0100.0000
static  const   int64_t    flagbit7 = 128;                         // 2^^7                          0000.0000..1000.0000

static  const   int64_t    flagbit8 = 256;                         // 2^^8                          0000.0001..0000.0000
static  const   int64_t    flagbit9 = 512;                         // 2^^9                          0000.0010..0000.0000
static  const   int64_t    flagbit10 = 1024;                       // 2^^10                         0000.0100..0000.0000
static  const   int64_t    flagbit11 = 2048;                       // 2^^11                         0000.1000..0000.0000

static  const   int64_t    flagbit12 = 4096;                       // 2^^12                         0001.0000..0000.0000
static  const   int64_t    flagbit13 = 8192;                       // 2^^13                         0010.0000..0000.0000
static  const   int64_t    flagbit14 = 16384;                      // 2^^14                         0100.0000..0000.0000
static  const   int64_t    flagbit15 = 32768;                      // 2^^15                         1000.0000..0000.0000

static const    int64_t	   flagbit17 = 131072;					   // 2^^15							1000.0000..0000.0000

static	const   int64_t    flagbit24 = 16777216;
static	const   int64_t    flagbit25 = 33554432;
static	const   int64_t    flagbit26 = 67108864;
static	const   int64_t    flagbit27 = 134217728;

// ------------------------------------------------------------------------------------------------
struct MATRIX
{
	double _11, _12, _13;
	double _21, _22, _23;
	double _31, _32, _33;
	double _41, _42, _43;
};

// ------------------------------------------------------------------------------------------------
struct OGLMATRIX
{
	double _11, _12, _13, _14;
	double _21, _22, _23, _24;
	double _31, _32, _33, _34;
	double _41, _42, _43, _44;
};

// ------------------------------------------------------------------------------------------------
struct VECTOR3
{
	double x;
	double y;
	double z;
};

// ------------------------------------------------------------------------------------------------
static	inline	void Vec3Transform(
	VECTOR3* pOut,
	const VECTOR3* pV,
	const MATRIX* pM
)
{
	VECTOR3	pTmp;

	pTmp.x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pM->_41;
	pTmp.y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pM->_42;
	pTmp.z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pM->_43;

	pOut->x = pTmp.x;
	pOut->y = pTmp.y;
	pOut->z = pTmp.z;
}

// ------------------------------------------------------------------------------------------------
static	inline	void Vec3Subtract(
	VECTOR3* pOut,
	const VECTOR3* pV1,
	const VECTOR3* pV2
)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;
}


// ------------------------------------------------------------------------------------------------
static void	Transform(const VECTOR3* pV, const MATRIX* pM, VECTOR3* pOut)
{
	VECTOR3	pTmp;
	pTmp.x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pM->_41;
	pTmp.y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pM->_42;
	pTmp.z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pM->_43;

	pOut->x = pTmp.x;
	pOut->y = pTmp.y;
	pOut->z = pTmp.z;
}

// ------------------------------------------------------------------------------------------------
static void MatrixIdentity(MATRIX* pOut)
{
	pOut->_12 = pOut->_13 =
		pOut->_21 = pOut->_23 =
		pOut->_31 = pOut->_32 =
		pOut->_41 = pOut->_42 = pOut->_43 = 0.;

	pOut->_11 = pOut->_22 = pOut->_33 = 1.;
}

// ------------------------------------------------------------------------------------------------
static void OGLMatrixIdentity(OGLMATRIX* pOut)
{
	memset(pOut, 0, sizeof(OGLMATRIX));

	pOut->_11 = pOut->_22 = pOut->_33 = pOut->_44 = 1.;
}
 
// ------------------------------------------------------------------------------------------------
static void	OGLTransform(const VECTOR3* pV, const OGLMATRIX* pM, VECTOR3* pOut)
{
	VECTOR3	pTmp;
	pTmp.x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pM->_41;
	pTmp.y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pM->_42;
	pTmp.z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pM->_43;

	pOut->x = pTmp.x;
	pOut->y = pTmp.y;
	pOut->z = pTmp.z;
}

// ------------------------------------------------------------------------------------------------
static void	MatrixMultiply(
	MATRIX* pOut,
	const MATRIX* pM1,
	const MATRIX* pM2
)
{
	ASSERT(pOut && pM1 && pM2);

	MATRIX pTmp;

	if (pOut) {
		pTmp._11 = pM1->_11 * pM2->_11 + pM1->_12 * pM2->_21 + pM1->_13 * pM2->_31;
		pTmp._12 = pM1->_11 * pM2->_12 + pM1->_12 * pM2->_22 + pM1->_13 * pM2->_32;
		pTmp._13 = pM1->_11 * pM2->_13 + pM1->_12 * pM2->_23 + pM1->_13 * pM2->_33;

		pTmp._21 = pM1->_21 * pM2->_11 + pM1->_22 * pM2->_21 + pM1->_23 * pM2->_31;
		pTmp._22 = pM1->_21 * pM2->_12 + pM1->_22 * pM2->_22 + pM1->_23 * pM2->_32;
		pTmp._23 = pM1->_21 * pM2->_13 + pM1->_22 * pM2->_23 + pM1->_23 * pM2->_33;

		pTmp._31 = pM1->_31 * pM2->_11 + pM1->_32 * pM2->_21 + pM1->_33 * pM2->_31;
		pTmp._32 = pM1->_31 * pM2->_12 + pM1->_32 * pM2->_22 + pM1->_33 * pM2->_32;
		pTmp._33 = pM1->_31 * pM2->_13 + pM1->_32 * pM2->_23 + pM1->_33 * pM2->_33;

		pTmp._41 = pM1->_41 * pM2->_11 + pM1->_42 * pM2->_21 + pM1->_43 * pM2->_31 + pM2->_41;
		pTmp._42 = pM1->_41 * pM2->_12 + pM1->_42 * pM2->_22 + pM1->_43 * pM2->_32 + pM2->_42;
		pTmp._43 = pM1->_41 * pM2->_13 + pM1->_42 * pM2->_23 + pM1->_43 * pM2->_33 + pM2->_43;

		pOut->_11 = pTmp._11;
		pOut->_12 = pTmp._12;
		pOut->_13 = pTmp._13;

		pOut->_21 = pTmp._21;
		pOut->_22 = pTmp._22;
		pOut->_23 = pTmp._23;

		pOut->_31 = pTmp._31;
		pOut->_32 = pTmp._32;
		pOut->_33 = pTmp._33;

		pOut->_41 = pTmp._41;
		pOut->_42 = pTmp._42;
		pOut->_43 = pTmp._43;
	}
	else {
		ASSERT(false);
	}
}

// ------------------------------------------------------------------------------------------------
static int64_t	GetDatatypeProperty__INTEGER(int64_t owlInstance, char* propertyName)
{
	int64_t* values = nullptr,
		card = 0;
	GetDatatypeProperty(
		owlInstance,
		GetPropertyByName(
			GetModel(
				owlInstance
			),
			propertyName
		),
		(void**)&values,
		&card
	);
	return	(card == 1) ? values[0] : 0;
}

// ------------------------------------------------------------------------------------------------
static double GetDatatypeProperty__DOUBLE(int64_t owlInstance, char* propertyName)
{
	double* values = nullptr;
	int64_t	card = 0;
	GetDatatypeProperty(
		owlInstance,
		GetPropertyByName(
			GetModel(
				owlInstance
			),
			propertyName
		),
		(void**)&values,
		&card
	);
	return	(card == 1) ? values[0] : 0.;
}

// ------------------------------------------------------------------------------------------------
static void SetDatatypeProperty__DOUBLE(int64_t owlInstance, char* propertyName, double value)
{
	SetDatatypeProperty(
		owlInstance,
		GetPropertyByName(
			GetModel(
				owlInstance
			),
			propertyName
		),
		&value,
		1
	);
}

// ------------------------------------------------------------------------------------------------
static int64_t	GetObjectProperty(int64_t owlInstance, char* propertyName)
{
	int64_t* values = nullptr,
		card = 0;
	GetObjectProperty(
		owlInstance,
		GetPropertyByName(
			GetModel(
				owlInstance
			),
			propertyName
		),
		&values,
		&card
	);
	return	(card == 1) ? values[0] : 0;
}

// ------------------------------------------------------------------------------------------------
static void SetObjectProperty(int64_t owlInstance, char* propertyName, int64_t owlInstanceObject)
{
	SetObjectProperty(
		owlInstance,
		GetPropertyByName(
			GetModel(
				owlInstance
			),
			propertyName
		),
		&owlInstanceObject,
		1
	);
}