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