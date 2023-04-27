#include "stdafx.h"
#include "IFCInstance.h"
#include "IFCModel.h"

#include <math.h>

// ------------------------------------------------------------------------------------------------
CIFCInstance::CIFCInstance(CIFCModel * pModel, SdaiInstance iInstance, const wchar_t * szGUID)
	: CInstance()
	, m_pModel(pModel)
	, m_iInstance(iInstance)
	, m_strGUID(szGUID != nullptr ? szGUID : L"")
	, m_iExpressID(0)
	, m_iParentExpressID(0)
	, m_iID(0)
	, m_pVertexBuffer(nullptr)
	, m_pIndexBuffer(nullptr)
	, m_iConceptualFacesCount(0)
	, m_vecTriangles()
	, m_vecConcFacePolygons()
	, m_vecLines()
	, m_vecPoints()
	, m_vecConcFacesCohorts()
	, m_vecConcFacePolygonsCohorts()
	, m_vecLinesCohorts()
	, m_vecPointsCohorts()
	, m_bReferenced(false)
	, m_bEnable(true)
	, m_iVBO(0)
	, m_iVBOOffset(0)
{
	m_iExpressID = internalGetP21Line(m_iInstance);

	{
		int_t iInstanceRelAggregates = 0;
		sdaiGetAttrBN(m_iInstance, "Decomposes", sdaiINSTANCE, &iInstanceRelAggregates);
		if (iInstanceRelAggregates) {
			int_t iInstanceParent = 0;
			sdaiGetAttrBN(iInstanceRelAggregates, "RelatingObject", sdaiINSTANCE, &iInstanceParent);
			if (iInstanceParent) {
				m_iParentExpressID = internalGetP21Line(iInstanceParent);
			}
		}
	}
	
	{
		int_t iInstanceRelContainedInSpatialStructure = 0;
		sdaiGetAttrBN(m_iInstance, "ContainedInStructure", sdaiINSTANCE, &iInstanceRelContainedInSpatialStructure);
		if (iInstanceRelContainedInSpatialStructure) {
			int_t iInstanceParent = 0;
			sdaiGetAttrBN(iInstanceRelContainedInSpatialStructure, "RelatingStructure", sdaiINSTANCE, &iInstanceParent);
			if (iInstanceParent) {
				m_iParentExpressID = internalGetP21Line(iInstanceParent);
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
CIFCInstance::~CIFCInstance()
{
	Clean();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ int64_t CIFCInstance::GetInstance() const
{
	return m_iInstance;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ bool CIFCInstance::HasGeometry() const
{
	return (m_pVertexBuffer != nullptr) && (m_pVertexBuffer->size() > 0);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ bool CIFCInstance::IsEnabled() const
{
	return GetEnable();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CIFCInstance::GetGUID() const
{
	return m_strGUID.c_str();
}

// ------------------------------------------------------------------------------------------------
int_t CIFCInstance::ExpressID() const
{
	return m_iExpressID;
}

// ------------------------------------------------------------------------------------------------
int_t CIFCInstance::ParentExpressID() const
{
	return m_iParentExpressID;
}

// ------------------------------------------------------------------------------------------------
int_t& CIFCInstance::ID()
{
	return m_iID;
}

CIFCModel * CIFCInstance::GetModel() const
{
	return m_pModel;
}

// ------------------------------------------------------------------------------------------------
float* CIFCInstance::GetVertices()
{
	if (m_pVertexBuffer != nullptr)
	{
		return m_pVertexBuffer->data();
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
int_t CIFCInstance::GetVerticesCount()
{
	if (m_pVertexBuffer != nullptr)
	{
		return m_pVertexBuffer->size();
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
int64_t CIFCInstance::GetVertexLength() const
{
	return VERTEX_LENGTH;
}

// ------------------------------------------------------------------------------------------------
int32_t* CIFCInstance::GetIndices() const
{
	if (m_pIndexBuffer != nullptr)
	{
		return m_pIndexBuffer->data();
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
int64_t CIFCInstance::GetIndicesCount() const
{
	if (m_pIndexBuffer != nullptr)
	{
		return m_pIndexBuffer->size();
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
int_t & CIFCInstance::conceptualFacesCount()
{
	return m_iConceptualFacesCount;
}

// ------------------------------------------------------------------------------------------------
const vector<_primitives >& CIFCInstance::GetTriangles() const
{
	return m_vecTriangles;
}

// ------------------------------------------------------------------------------------------------
const vector<_primitives >& CIFCInstance::GetLines() const
{
	return m_vecLines;
}

// ------------------------------------------------------------------------------------------------
const vector<_primitives >& CIFCInstance::GetPoints() const
{
	return m_vecPoints;
}

// ------------------------------------------------------------------------------------------------
const vector<_primitives >& CIFCInstance::GetConcFacesPolygons() const
{
	return m_vecConcFacePolygons;
}

// ------------------------------------------------------------------------------------------------
vector<_facesCohort*>& CIFCInstance::ConcFacesCohorts()
{
	return m_vecConcFacesCohorts;
}

vector<_cohort*>& CIFCInstance::ConcFacePolygonsCohorts()
{
	return m_vecConcFacePolygonsCohorts;
}

// ------------------------------------------------------------------------------------------------
vector<_cohort*>& CIFCInstance::LinesCohorts()
{
	return m_vecLinesCohorts;
}

// ------------------------------------------------------------------------------------------------
vector<_facesCohort*>& CIFCInstance::PointsCohorts()
{
	return m_vecPointsCohorts;
}

// ------------------------------------------------------------------------------------------------
bool&  CIFCInstance::Referenced()
{
	return m_bReferenced;
}

// ------------------------------------------------------------------------------------------------
bool CIFCInstance::GetEnable() const
{
	return m_bEnable;
}

// ------------------------------------------------------------------------------------------------
void CIFCInstance::SetEnable(bool bEnable)
{
	m_bEnable = bEnable;
}

// ------------------------------------------------------------------------------------------------
GLuint& CIFCInstance::VBO()
{
	return m_iVBO;
}

// ------------------------------------------------------------------------------------------------
GLsizei& CIFCInstance::VBOOffset()
{
	return m_iVBOOffset;
}

// ------------------------------------------------------------------------------------------------
void CIFCInstance::CalculateMinMax(
	float& fXmin, float& fXmax, 
	float& fYmin, float& fYmax, 
	float& fZmin, float& fZmax)
{
	
	if (!HasGeometry())
	{
		return;
	}

	/*
	* Triangles
	*/
	if (!m_vecTriangles.empty())
	{
		for (size_t iTriangle = 0; iTriangle < m_vecTriangles.size(); iTriangle++)
		{
			for (int64_t iIndex = m_vecTriangles[iTriangle].startIndex();
				iIndex < m_vecTriangles[iTriangle].startIndex() + m_vecTriangles[iTriangle].indicesCount();
				iIndex++)
			{
				fXmin = (float)fmin(fXmin, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH)]);
				fXmax = (float)fmax(fXmax, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH)]);
				fYmin = (float)fmin(fYmin, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iTriangle = ...
	} // if (!m_vecTriangles.empty())	

	/*
	* Conceptual faces polygons
	*/
	if (!m_vecConcFacePolygons.empty())
	{
		for (size_t iPolygon = 0; iPolygon < m_vecConcFacePolygons.size(); iPolygon++)
		{
			for (int64_t iIndex = m_vecConcFacePolygons[iPolygon].startIndex();
				iIndex < m_vecConcFacePolygons[iPolygon].startIndex() + m_vecConcFacePolygons[iPolygon].indicesCount();
				iIndex++)
			{
				if ((GetIndices()[iIndex] == -1) || (GetIndices()[iIndex] == -2))
				{
					continue;
				}

				fXmin = (float)fmin(fXmin, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH)]);
				fXmax = (float)fmax(fXmax, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH)]);
				fYmin = (float)fmin(fYmin, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPolygon = ...
	} // if (!m_vecConcFacePolygons.empty())

	/*
	* Lines
	*/
	if (!m_vecLines.empty())
	{
		for (size_t iPolygon = 0; iPolygon < m_vecLines.size(); iPolygon++)
		{
			for (int64_t iIndex = m_vecLines[iPolygon].startIndex();
				iIndex < m_vecLines[iPolygon].startIndex() + m_vecLines[iPolygon].indicesCount();
				iIndex++)
			{
				if (GetIndices()[iIndex] == -1)
				{
					continue;
				}

				fXmin = (float)fmin(fXmin, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH)]);
				fXmax = (float)fmax(fXmax, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH)]);
				fYmin = (float)fmin(fYmin, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPolygon = ...
	} // if (!m_vecLines.empty())

	/*
	* Points
	*/
	if (!m_vecPoints.empty())
	{
		for (size_t iPolygon = 0; iPolygon < m_vecPoints.size(); iPolygon++)
		{
			for (int64_t iIndex = m_vecPoints[iPolygon].startIndex();
				iIndex < m_vecPoints[iPolygon].startIndex() + m_vecPoints[iPolygon].indicesCount();
				iIndex++)
			{
				fXmin = (float)fmin(fXmin, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH)]);
				fXmax = (float)fmax(fXmax, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH)]);
				fYmin = (float)fmin(fYmin, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, GetVertices()[(GetIndices()[iIndex] * VERTEX_LENGTH) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPolygon = ...
	} // if (!m_vecPoints.empty())
}

// ------------------------------------------------------------------------------------------------
void CIFCInstance::ScaleAndCenter(
	float fXmin, float fXmax, 
	float fYmin, float fYmax, 
	float fZmin, float fZmax, 
	float fResoltuion,
	bool bScale)
{
	if (!HasGeometry())
	{
		return;
	}

	/**
	* Vertices
	*/
	for (int_t iVertex = 0; iVertex < m_pVertexBuffer->size(); iVertex++)
	{
		// [0.0 -> X/Y/Zmin + X/Y/Zmax]
		GetVertices()[(iVertex * VERTEX_LENGTH)] = GetVertices()[(iVertex * VERTEX_LENGTH)] - fXmin;
		GetVertices()[(iVertex * VERTEX_LENGTH) + 1] = GetVertices()[(iVertex * VERTEX_LENGTH) + 1] - fYmin;
		GetVertices()[(iVertex * VERTEX_LENGTH) + 2] = GetVertices()[(iVertex * VERTEX_LENGTH) + 2] - fZmin;

		// center
		GetVertices()[(iVertex * VERTEX_LENGTH)] = GetVertices()[(iVertex * VERTEX_LENGTH)] - ((fXmax - fXmin) / 2.0f);
		GetVertices()[(iVertex * VERTEX_LENGTH) + 1] = GetVertices()[(iVertex * VERTEX_LENGTH) + 1] - ((fYmax - fYmin) / 2.0f);
		GetVertices()[(iVertex * VERTEX_LENGTH) + 2] = GetVertices()[(iVertex * VERTEX_LENGTH) + 2] - ((fZmax - fZmin) / 2.0f);

		// [-1.0 -> 1.0]
		if (bScale)
		{
			GetVertices()[(iVertex * VERTEX_LENGTH)] = GetVertices()[(iVertex * VERTEX_LENGTH)] / (fResoltuion / 2.0f);
			GetVertices()[(iVertex * VERTEX_LENGTH) + 1] = GetVertices()[(iVertex * VERTEX_LENGTH) + 1] / (fResoltuion / 2.0f);
			GetVertices()[(iVertex * VERTEX_LENGTH) + 2] = GetVertices()[(iVertex * VERTEX_LENGTH) + 2] / (fResoltuion / 2.0f);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCInstance::Clean()
{
	delete m_pVertexBuffer;
	m_pVertexBuffer = nullptr;

	delete m_pIndexBuffer;
	m_pIndexBuffer = nullptr;

	m_iConceptualFacesCount = 0;

	m_vecTriangles.clear();
	m_vecConcFacePolygons.clear();
	m_vecLines.clear();
	m_vecPoints.clear();

	_cohort::clear(m_vecConcFacesCohorts);
	_cohort::clear(m_vecConcFacePolygonsCohorts);
	_cohort::clear(m_vecLinesCohorts);
	_cohort::clear(m_vecPointsCohorts);
}