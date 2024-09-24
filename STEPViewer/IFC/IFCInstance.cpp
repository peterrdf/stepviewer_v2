#include "stdafx.h"
#include "IFCInstance.h"
#include "IFCModel.h"

#include <math.h>

// ************************************************************************************************
CIFCInstance::CIFCInstance(int64_t iID, SdaiInstance iSdaiInstance)
	: _geometry(iID, iSdaiInstance, true)
	, CInstanceBase()
	, m_iExpressID(internalGetP21Line(iSdaiInstance))
	, m_bReferenced(false)
{
	ASSERT(iSdaiInstance != 0);
	ASSERT(m_iExpressID != 0);

	Calculate();
}

CIFCInstance::~CIFCInstance()
{}

/*virtual*/ OwlModel CIFCInstance::getModel() const /*override*/
{
	OwlModel iOwlModel = 0;
	owlGetModel(GetModel(), &iOwlModel);
	ASSERT(iOwlModel != 0);

	return iOwlModel;
}

/*virtual*/ int64_t CIFCInstance::calculateInstance(int64_t* piVertexBufferSize, int64_t* piIndexBufferSize) /*override*/
{
	assert(piVertexBufferSize != nullptr);
	assert(piIndexBufferSize != nullptr);

	*piVertexBufferSize = *piIndexBufferSize = 0;

	SdaiModel iSdaiModel = sdaiGetInstanceModel((int_t)m_iInstance);

	int64_t iOwlInstance = 0;
	owlBuildInstance(iSdaiModel, (int_t)m_iInstance, &iOwlInstance);

	CalculateInstance(iOwlInstance, piVertexBufferSize, piIndexBufferSize, nullptr);

	return iOwlInstance;
}

void CIFCInstance::CalculateMinMax(
	float& fXmin, float& fXmax, 
	float& fYmin, float& fYmax, 
	float& fZmin, float& fZmax)
{
	
	if (!HasGeometry())
	{
		return;
	}

	const uint32_t VERTEX_LENGTH = getVertexLength();

	// Triangles
	if (!m_vecTriangles.empty())
	{
		for (size_t iPrimitive = 0; iPrimitive < m_vecTriangles.size(); iPrimitive++)
		{
			for (int64_t iIndex = m_vecTriangles[iPrimitive].startIndex();
				iIndex < m_vecTriangles[iPrimitive].startIndex() + m_vecTriangles[iPrimitive].indicesCount();
				iIndex++)
			{
				fXmin = (float)fmin(fXmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 0]);
				fXmax = (float)fmax(fXmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 0]);
				fYmin = (float)fmin(fYmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPrimitive = ...
	} // if (!m_vecTriangles.empty())	

	// Conceptual faces polygons
	if (!m_vecConcFacePolygons.empty())
	{
		for (size_t iPrimitive = 0; iPrimitive < m_vecConcFacePolygons.size(); iPrimitive++)
		{
			for (int64_t iIndex = m_vecConcFacePolygons[iPrimitive].startIndex();
				iIndex < m_vecConcFacePolygons[iPrimitive].startIndex() + m_vecConcFacePolygons[iPrimitive].indicesCount();
				iIndex++)
			{
				if ((m_pIndexBuffer->data()[iIndex] == -1) || (m_pIndexBuffer->data()[iIndex] == -2))
				{
					continue;
				}

				fXmin = (float)fmin(fXmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 0]);
				fXmax = (float)fmax(fXmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 0]);
				fYmin = (float)fmin(fYmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPrimitive = ...
	} // if (!m_vecConcFacePolygons.empty())

	// Lines
	if (!m_vecLines.empty())
	{
		for (size_t iPrimitive = 0; iPrimitive < m_vecLines.size(); iPrimitive++)
		{
			for (int64_t iIndex = m_vecLines[iPrimitive].startIndex();
				iIndex < m_vecLines[iPrimitive].startIndex() + m_vecLines[iPrimitive].indicesCount();
				iIndex++)
			{
				if (m_pIndexBuffer->data()[iIndex] == -1)
				{
					continue;
				}

				fXmin = (float)fmin(fXmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 0]);
				fXmax = (float)fmax(fXmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 0]);
				fYmin = (float)fmin(fYmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPrimitive = ...
	} // if (!m_vecLines.empty())

	// Points
	if (!m_vecPoints.empty())
	{
		for (size_t iPrimitive = 0; iPrimitive < m_vecPoints.size(); iPrimitive++)
		{
			for (int64_t iIndex = m_vecPoints[iPrimitive].startIndex();
				iIndex < m_vecPoints[iPrimitive].startIndex() + m_vecPoints[iPrimitive].indicesCount();
				iIndex++)
			{
				fXmin = (float)fmin(fXmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 0]);
				fXmax = (float)fmax(fXmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 0]);
				fYmin = (float)fmin(fYmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * VERTEX_LENGTH) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPrimitive = ...
	} // if (!m_vecPoints.empty())
}

void CIFCInstance::Scale(float fScaleFactor)
{
	_geometry::scale(fScaleFactor);
}

void CIFCInstance::Calculate()
{
	// Format
	setSTEPFormatSettings();

	// Extra settings
	const int_t flagbit1 = 2;
	setFilter(GetModel(), flagbit1, flagbit1);
	setSegmentation(GetModel(), 16, 0.);

	/* Geometry */

	ASSERT(m_pVertexBuffer == nullptr);
	m_pVertexBuffer = new _vertices_f(getVertexLength());

	ASSERT(m_pIndexBuffer == nullptr);
	m_pIndexBuffer = new _indices_i32();

	calculate(m_pVertexBuffer, m_pIndexBuffer);

	MATERIALS mapMaterial2ConcFaces;
	MATERIALS mapMaterial2ConcFaceLines;
	MATERIALS mapMaterial2ConcFacePoints;

	m_iConceptualFacesCount = GetConceptualFaceCnt(m_iInstance);
	for (int64_t iConceptualFaceIndex = 0; iConceptualFaceIndex < m_iConceptualFacesCount; iConceptualFaceIndex++)
	{
		int64_t iStartIndexTriangles = 0;
		int64_t iIndicesCountTriangles = 0;
		int64_t iStartIndexLines = 0;
		int64_t iIndicesCountLines = 0;
		int64_t iStartIndexPoints = 0;
		int64_t iIndicesCountPoints = 0;
		int64_t iStartIndexConceptualFacePolygons = 0;
		int64_t iIndicesCountConceptualFacePolygons = 0;
		ConceptualFace iConceptualFace = GetConceptualFaceEx(
			m_iInstance,
			iConceptualFaceIndex,
			&iStartIndexTriangles, &iIndicesCountTriangles,
			&iStartIndexLines, &iIndicesCountLines,
			&iStartIndexPoints, &iIndicesCountPoints,
			0, 0,
			&iStartIndexConceptualFacePolygons, &iIndicesCountConceptualFacePolygons);

		/* Material */

		uint32_t iAmbientColor = CIFCModel::DEFAULT_COLOR;
		uint32_t iDiffuseColor = CIFCModel::DEFAULT_COLOR;
		uint32_t iEmissiveColor = CIFCModel::DEFAULT_COLOR;
		uint32_t iSpecularColor = CIFCModel::DEFAULT_COLOR;
		float fTransparency = 1.f;

		OwlInstance iMaterialInstance = GetConceptualFaceMaterial(iConceptualFace);
		if (iMaterialInstance != 0)
		{
			iAmbientColor = GetMaterialColorAmbient(iMaterialInstance);
			iDiffuseColor = GetMaterialColorDiffuse(iMaterialInstance);
			iEmissiveColor = GetMaterialColorEmissive(iMaterialInstance);
			iSpecularColor = GetMaterialColorSpecular(iMaterialInstance);
			fTransparency = (float)COLOR_GET_W(iAmbientColor);
		}

		_material material(
			iAmbientColor,
			iDiffuseColor,
			iEmissiveColor,
			iSpecularColor,
			fTransparency,
			nullptr);

		if (iIndicesCountTriangles > 0)
		{
			addTriangles(iConceptualFaceIndex, iStartIndexTriangles, iIndicesCountTriangles, material, mapMaterial2ConcFaces);
		}

		if (iIndicesCountConceptualFacePolygons > 0)
		{
			addConcFacePolygons(iStartIndexConceptualFacePolygons, iIndicesCountConceptualFacePolygons);
		}

		if (iIndicesCountLines > 0)
		{
			addLines(iConceptualFaceIndex, iStartIndexLines, iIndicesCountLines, material, mapMaterial2ConcFaceLines);
		}

		if (iIndicesCountPoints > 0)
		{
			addPoints(iConceptualFaceIndex, iStartIndexPoints, iIndicesCountPoints, material, mapMaterial2ConcFacePoints);
		}
	} // for (int64_t iConceptualFaceIndex = ...	

	// Build the cohorts
	buildConcFacesCohorts(mapMaterial2ConcFaces, _oglUtils::getIndicesCountLimit());
	buildConcFacePolygonsCohorts(_oglUtils::getIndicesCountLimit());
	buildLinesCohorts(mapMaterial2ConcFaceLines, _oglUtils::getIndicesCountLimit());
	buildLinesCohorts(mapMaterial2ConcFacePoints, _oglUtils::getIndicesCountLimit());
	buildPointsCohorts(mapMaterial2ConcFacePoints, _oglUtils::getIndicesCountLimit());
}
