#include "stdafx.h"
#include "AP242ProductDefinition.h"
#include "AP242Model.h"

// ************************************************************************************************
CAP242ProductDefinition::CAP242ProductDefinition(SdaiInstance iSdaiInstance)
	: _ap_geometry(iSdaiInstance)
	, m_iExpressID(internalGetP21Line(iSdaiInstance))
	, m_szId(nullptr)
	, m_szName(nullptr)
	, m_szDescription(nullptr)
	, m_szProductId(nullptr)
	, m_szProductName(nullptr)
	, m_iRelatingProducts(0)
	, m_iRelatedProducts(0)
	, m_vecInstances()
	, m_iNextInstance(-1)
{
	ASSERT(m_iExpressID != 0);

	sdaiGetAttrBN(iSdaiInstance, "id", sdaiUNICODE, &m_szId);
	sdaiGetAttrBN(iSdaiInstance, "name", sdaiUNICODE, &m_szName);
	sdaiGetAttrBN(iSdaiInstance, "description", sdaiUNICODE, &m_szDescription);

	SdaiInstance iFormationInstance = 0;
	sdaiGetAttrBN(iSdaiInstance, "formation", sdaiINSTANCE, &iFormationInstance);
	ASSERT(iFormationInstance != 0);

	SdaiInstance iProductInstance = 0;
	sdaiGetAttrBN(iFormationInstance, "of_product", sdaiINSTANCE, &iProductInstance);

	sdaiGetAttrBN(iProductInstance, "id", sdaiUNICODE, &m_szProductId);
	sdaiGetAttrBN(iProductInstance, "name", sdaiUNICODE, &m_szProductName);

	calculate();
}

/*virtual*/ CAP242ProductDefinition::~CAP242ProductDefinition()
{}

/*virtual*/ void CAP242ProductDefinition::preCalculate() /*override*/
{
	// Format
	setSTEPFormatSettings();

	// Extra settings
	setSegmentation(GetModel(), 16, 0.);
}

/*virtual*/ void CAP242ProductDefinition::postCalculate() /*override*/
{
}

void CAP242ProductDefinition::CalculateMinMaxTransform(
	CAP242ProductInstance* pInstance,
	float fXTranslation, float fYTranslation, float fZTranslation,
	float& fXmin, float& fXmax,
	float& fYmin, float& fYmax,
	float& fZmin, float& fZmax)
{
	if (!hasGeometry())
	{
		return;
	}

	double _41 = pInstance->getTransformationMatrix()->_41;
	double _42 = pInstance->getTransformationMatrix()->_42;
	double _43 = pInstance->getTransformationMatrix()->_43;

	pInstance->getTransformationMatrix()->_41 += fXTranslation;
	pInstance->getTransformationMatrix()->_42 += fYTranslation;
	pInstance->getTransformationMatrix()->_43 += fZTranslation;

	CalculateMinMaxTransform(
		pInstance,
		fXmin, fXmax,
		fYmin, fYmax,
		fZmin, fZmax);

	pInstance->getTransformationMatrix()->_41 = _41;
	pInstance->getTransformationMatrix()->_42 = _42;
	pInstance->getTransformationMatrix()->_43 = _43;
}

void CAP242ProductDefinition::CalculateMinMaxTransform(
	CAP242ProductInstance* pInstance,
	float& fXmin, float& fXmax, 
	float& fYmin, float& fYmax, 
	float& fZmin, float& fZmax)
{
	if (!hasGeometry())
	{
		return;
	}

	const uint32_t VERTEX_LENGTH = getVertexLength();

	// Triangles
	if (!m_vecTriangles.empty())
	{
		for (size_t iTriangle = 0; iTriangle < m_vecTriangles.size(); iTriangle++)
		{
			for (int64_t iIndex = m_vecTriangles[iTriangle].startIndex();
				iIndex < m_vecTriangles[iTriangle].startIndex() + m_vecTriangles[iTriangle].indicesCount();
				iIndex++)
			{
				_vector3 vecPoint =
				{
					m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 0],
					m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 1],
					m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 2]
				};

				if (pInstance != nullptr)
				{
					_transform(&vecPoint, pInstance->getTransformationMatrix(), &vecPoint);
				}				

				fXmin = (float)fmin(fXmin, vecPoint.x);
				fXmax = (float)fmax(fXmax, vecPoint.x);
				fYmin = (float)fmin(fYmin, vecPoint.y);
				fYmax = (float)fmax(fYmax, vecPoint.y);
				fZmin = (float)fmin(fZmin, vecPoint.z);
				fZmax = (float)fmax(fZmax, vecPoint.z);
			} // for (size_t iIndex = ...
		} // for (size_t iTriangle = ...
	} // if (!m_vecTriangles.empty())	

	// Conceptual faces polygons
	if (!m_vecConcFacePolygons.empty())
	{
		for (size_t iPolygon = 0; iPolygon < m_vecConcFacePolygons.size(); iPolygon++)
		{
			for (int64_t iIndex = m_vecConcFacePolygons[iPolygon].startIndex();
				iIndex < m_vecConcFacePolygons[iPolygon].startIndex() + m_vecConcFacePolygons[iPolygon].indicesCount();
				iIndex++)
			{
				if ((m_pIndexBuffer->data()[iIndex] == -1) || (m_pIndexBuffer->data()[iIndex] == -2))
				{
					continue;
				}

				_vector3 vecPoint =
				{
					m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 0],
					m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 1],
					m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 2]
				};

				if (pInstance != nullptr)
				{
					_transform(&vecPoint, pInstance->getTransformationMatrix(), &vecPoint);
				}				

				fXmin = (float)fmin(fXmin, vecPoint.x);
				fXmax = (float)fmax(fXmax, vecPoint.x);
				fYmin = (float)fmin(fYmin, vecPoint.y);
				fYmax = (float)fmax(fYmax, vecPoint.y);
				fZmin = (float)fmin(fZmin, vecPoint.z);
				fZmax = (float)fmax(fZmax, vecPoint.z);
			} // for (size_t iIndex = ...
		} // for (size_t iPolygon = ...
	} // if (!m_vecConcFacePolygons.empty())

	// Lines
	if (!m_vecLines.empty())
	{
		for (size_t iPolygon = 0; iPolygon < m_vecLines.size(); iPolygon++)
		{
			for (int64_t iIndex = m_vecLines[iPolygon].startIndex();
				iIndex < m_vecLines[iPolygon].startIndex() + m_vecLines[iPolygon].indicesCount();
				iIndex++)
			{
				if (m_pIndexBuffer->data()[iIndex] == -1)
				{
					continue;
				}

				_vector3 vecPoint =
				{
					m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 0],
					m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 1],
					m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 2]
				};

				if (pInstance != nullptr)
				{
					_transform(&vecPoint, pInstance->getTransformationMatrix(), &vecPoint);
				}				

				fXmin = (float)fmin(fXmin, vecPoint.x);
				fXmax = (float)fmax(fXmax, vecPoint.x);
				fYmin = (float)fmin(fYmin, vecPoint.y);
				fYmax = (float)fmax(fYmax, vecPoint.y);
				fZmin = (float)fmin(fZmin, vecPoint.z);
				fZmax = (float)fmax(fZmax, vecPoint.z);
			} // for (size_t iIndex = ...
		} // for (size_t iPolygon = ...
	} // if (!m_vecLines.empty())

	// Points
	if (!m_vecPoints.empty())
	{
		for (size_t iPolygon = 0; iPolygon < m_vecPoints.size(); iPolygon++)
		{
			for (int64_t iIndex = m_vecPoints[iPolygon].startIndex(); 
				iIndex < m_vecPoints[iPolygon].startIndex() + m_vecPoints[iPolygon].indicesCount();
				iIndex++)
			{
				_vector3 vecPoint =
				{
					m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 0],
					m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 1],
					m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 2]
				};

				if (pInstance != nullptr)
				{
					_transform(&vecPoint, pInstance->getTransformationMatrix(), &vecPoint);
				}				

				fXmin = (float)fmin(fXmin, vecPoint.x);
				fXmax = (float)fmax(fXmax, vecPoint.x);
				fYmin = (float)fmin(fYmin, vecPoint.y);
				fYmax = (float)fmax(fYmax, vecPoint.y);
				fZmin = (float)fmin(fZmin, vecPoint.z);
				fZmax = (float)fmax(fZmax, vecPoint.z);
			} // for (size_t iIndex = ...
		} // for (size_t iPolygon = ...
	} // if (!m_vecPoints.empty())
}

void CAP242ProductDefinition::Scale(float fScaleFactor)
{
	if (getVerticesCount() == 0)
	{
		return;
	}

	// Geometry
	_geometry::scale(fScaleFactor);

	// Instances
	for (size_t iInstance = 0; iInstance < m_vecInstances.size(); iInstance++)
	{
		m_vecInstances[iInstance]->scale(fScaleFactor);
	}
}

int32_t CAP242ProductDefinition::GetNextInstance()
{
	if (++m_iNextInstance >= (int32_t)m_vecInstances.size())
	{
		m_iNextInstance = 0;
	}

	return m_iNextInstance;
}
