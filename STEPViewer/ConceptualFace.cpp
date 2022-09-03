#include "stdafx.h"
#include "ConceptualFace.h"

// ------------------------------------------------------------------------------------------------
CConceptualFace::CConceptualFace()
	: m_iIndex(-1)
	, m_iTrianglesStartIndex(0)
	, m_iTrianglesIndicesCount(0)
{
}

// ------------------------------------------------------------------------------------------------
CConceptualFace::~CConceptualFace()
{
}

// ------------------------------------------------------------------------------------------------
int_t & CConceptualFace::index()
{
	return m_iIndex;
}

// ------------------------------------------------------------------------------------------------
int_t & CConceptualFace::trianglesStartIndex()
{
	return m_iTrianglesStartIndex;
}

// ------------------------------------------------------------------------------------------------
int_t & CConceptualFace::trianglesIndicesCount()
{
	return m_iTrianglesIndicesCount;
}
