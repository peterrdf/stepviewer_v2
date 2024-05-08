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