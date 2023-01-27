#include "stdafx.h"
#include "IFCObject.h"
#include "IFCModel.h"

#include <math.h>

// ------------------------------------------------------------------------------------------------
CIFCObject::CIFCObject(CIFCModel * pIFCModel, int_t iInstance, const wchar_t * szGUID, int_t iEntity, const wchar_t * szEntity)
	: CSTEPInstance()
	, m_pIFCModel(pIFCModel)
	, m_iInstance(iInstance)
	, m_strGUID(szGUID)
	, m_iEntity(iEntity)
	, m_strEntity(szEntity)
	, m_iExpressID(0)
	, m_iParentExpressID(0)
	, m_iID(0)
	, m_pVertexBuffer(NULL)
	, m_pIndexBuffer(NULL)
	, m_iConceptualFacesCount(0)
	, m_vecTriangles()
	, m_vecConcFacePolygons()
	, m_vecLines()
	, m_vecPoints()
	, m_vecConcFacesCohorts()
	, m_vecConcFacePolygonsCohorts()
	, m_vecLinesCohorts()
	, m_vecPointsCohorts()
	, m_prXMinMax(pair<float, float>(-1.f, 1.f))
	, m_prYMinMax(pair<float, float>(-1.f, 1.f))
	, m_prZMinMax(pair<float, float>(-1.f, 1.f))
	, m_bReferenced(false)
	, m_bEnable(true)
	, m_bSelected(false)
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
CIFCObject::~CIFCObject()
{
	Clean();
}

// ------------------------------------------------------------------------------------------------
int_t CIFCObject::getInstance() const
{
	return m_iInstance;
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CIFCObject::getGUID() const
{
	return m_strGUID.c_str();
}

// ------------------------------------------------------------------------------------------------
int_t CIFCObject::getEntity() const
{
	return m_iEntity;
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CIFCObject::getEntityName() const
{
	return m_strEntity.c_str();
}

// ------------------------------------------------------------------------------------------------
int_t CIFCObject::expressID() const
{
	return m_iExpressID;
}

// ------------------------------------------------------------------------------------------------
int_t CIFCObject::parentExpressID() const
{
	return m_iParentExpressID;
}

// ------------------------------------------------------------------------------------------------
int_t & CIFCObject::ID()
{
	return m_iID;
}

// ------------------------------------------------------------------------------------------------
bool CIFCObject::hasGeometry() const
{
	return (m_pVertexBuffer != NULL) && (m_pVertexBuffer->size() > 0);
}

CIFCModel * CIFCObject::GetModel() const
{
	return m_pIFCModel;
}

// ------------------------------------------------------------------------------------------------
float* CIFCObject::getVertices()
{
	if (m_pVertexBuffer != nullptr)
	{
		return m_pVertexBuffer->data();
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
int_t CIFCObject::getVerticesCount()
{
	if (m_pVertexBuffer != nullptr)
	{
		return m_pVertexBuffer->size();
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
int64_t CIFCObject::getVertexLength() const
{
	return VERTEX_LENGTH;
}

// ------------------------------------------------------------------------------------------------
int32_t* CIFCObject::getIndices() const
{
	if (m_pIndexBuffer != nullptr)
	{
		return m_pIndexBuffer->data();
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
int64_t CIFCObject::getIndicesCount() const
{
	if (m_pIndexBuffer != nullptr)
	{
		return m_pIndexBuffer->size();
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
int_t & CIFCObject::conceptualFacesCount()
{
	return m_iConceptualFacesCount;
}

// ------------------------------------------------------------------------------------------------
const vector<_primitives >& CIFCObject::getTriangles() const
{
	return m_vecTriangles;
}

// ------------------------------------------------------------------------------------------------
const vector<_primitives >& CIFCObject::getLines() const
{
	return m_vecLines;
}

// ------------------------------------------------------------------------------------------------
const vector<_primitives >& CIFCObject::getPoints() const
{
	return m_vecPoints;
}

// ------------------------------------------------------------------------------------------------
const vector<_primitives >& CIFCObject::getConcFacesPolygons() const
{
	return m_vecConcFacePolygons;
}

// ------------------------------------------------------------------------------------------------
vector<_facesCohort*>& CIFCObject::concFacesCohorts()
{
	return m_vecConcFacesCohorts;
}

vector<_cohort*>& CIFCObject::concFacePolygonsCohorts()
{
	return m_vecConcFacePolygonsCohorts;
}

// ------------------------------------------------------------------------------------------------
vector<_cohort*>& CIFCObject::linesCohorts()
{
	return m_vecLinesCohorts;
}

// ------------------------------------------------------------------------------------------------
vector<_facesCohort*>& CIFCObject::pointsCohorts()
{
	return m_vecPointsCohorts;
}

// ------------------------------------------------------------------------------------------------
const pair<float, float> & CIFCObject::getXMinMax() const
{
	return m_prXMinMax;
}

// ------------------------------------------------------------------------------------------------
const pair<float, float> & CIFCObject::getYMinMax() const
{
	return m_prYMinMax;
}

// ------------------------------------------------------------------------------------------------
const pair<float, float> & CIFCObject::getZMinMax() const
{
	return m_prZMinMax;
}

// ------------------------------------------------------------------------------------------------
bool&  CIFCObject::referenced()
{
	return m_bReferenced;
}

// ------------------------------------------------------------------------------------------------
bool CIFCObject::getEnable() const
{
	return m_bEnable;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::setEnable(bool bEnable)
{
	m_bEnable = bEnable;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::setSelected(bool bSelected)
{
	m_bSelected = bSelected;
}

// ------------------------------------------------------------------------------------------------
bool CIFCObject::getSelected() const
{
	return m_bSelected;
}

// ------------------------------------------------------------------------------------------------
GLuint & CIFCObject::VBO()
{
	return m_iVBO;
}

// ------------------------------------------------------------------------------------------------
GLsizei & CIFCObject::VBOOffset()
{
	return m_iVBOOffset;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::CalculateMinMaxValues(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax)
{
	if (getVerticesCount() == 0)
	{
		return;
	}

	for (int_t iVertex = 0; iVertex < getVerticesCount(); iVertex++)
	{
		fXmin = fmin(fXmin, m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)]);
		fXmax = fmax(fXmax, m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)]);

		fYmin = fmin(fYmin, m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1]);
		fYmax = fmax(fYmax, m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1]);

		fZmin = fmin(fZmin, m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2]);
		fZmax = fmax(fZmax, m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2]);
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::CalculateMinMaxValues(float* pVertices, int_t iVerticesCount)
{
	float fXmin = FLT_MAX;
	float fXmax = -FLT_MAX;
	float fYmin = FLT_MAX;
	float fYmax = -FLT_MAX;
	float fZmin = FLT_MAX;
	float fZmax = -FLT_MAX;

	for (int_t iVertex = 0; iVertex < iVerticesCount; iVertex++)
	{
		fXmin = fmin(fXmin, pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)]);
		fXmax = fmax(fXmax, pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)]);

		fYmin = fmin(fYmin, pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1]);
		fYmax = fmax(fYmax, pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1]);

		fZmin = fmin(fZmin, pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2]);
		fZmax = fmax(fZmax, pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2]);
	}

	m_prXMinMax = pair<float, float>(fXmin, fXmax);
	m_prYMinMax = pair<float, float>(fYmin, fYmax);
	m_prZMinMax = pair<float, float>(fZmin, fZmax);
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::Scale(float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion)
{
	if (getVerticesCount() == 0)
	{
		ASSERT(FALSE);

		return;
	}

	for (int_t iVertex = 0; iVertex < getVerticesCount(); iVertex++)
	{
		// [0.0 -> X/Y/Zmin + X/Y/Zmax]
		m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] = m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] - fXmin;
		m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] = m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] - fYmin;
		m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] = m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] - fZmin;

		// center
		m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] = m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] - ((fXmax - fXmin) / 2.0f);
		m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] = m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] - ((fYmax - fYmin) / 2.0f);
		m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] = m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] - ((fZmax - fZmin) / 2.0f);

		// [-1.0 -> 1.0]
		m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] = m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] / (fResoltuion / 2.0f);
		m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] = m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] / (fResoltuion / 2.0f);
		m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] = m_pVertexBuffer->data()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] / (fResoltuion / 2.0f);
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::Scale(float * pVertices, int_t iVerticesCount, float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion)
{
	if ((pVertices == 0) || (iVerticesCount == 0))
	{
		ASSERT(FALSE);

		return;
	}

	for (int_t iVertex = 0; iVertex < iVerticesCount; iVertex++)
	{
		// [0.0 -> X/Y/Zmin + X/Y/Zmax]
		pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] = pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] - fXmin;
		pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] = pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] - fYmin;
		pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] = pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] - fZmin;

		// center
		pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] = pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] - ((fXmax - fXmin) / 2.0f);
		pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] = pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] - ((fYmax - fYmin) / 2.0f);
		pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] = pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] - ((fZmax - fZmin) / 2.0f);

		// [-1.0 -> 1.0]
		pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] = pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] / (fResoltuion / 2.0f);
		pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] = pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] / (fResoltuion / 2.0f);
		pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] = pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] / (fResoltuion / 2.0f);
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::Clean()
{
	delete m_pVertexBuffer;
	m_pVertexBuffer = NULL;

	delete m_pIndexBuffer;
	m_pIndexBuffer = NULL;

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