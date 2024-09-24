#include "stdafx.h"
#include "ProductDefinition.h"
#include "STEPModel.h"

// ************************************************************************************************
CProductDefinition::CProductDefinition(SdaiInstance iSdaiInstance)
	: _geometry(-1, iSdaiInstance, true)
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
	ASSERT(iSdaiInstance != 0);
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

	Calculate();
}

/*virtual*/ CProductDefinition::~CProductDefinition()
{}

/*virtual*/ OwlModel CProductDefinition::getModel() const /*override*/
{
	OwlModel iOwlModel = 0;
	owlGetModel(GetModel(), &iOwlModel);
	ASSERT(iOwlModel != 0);

	return iOwlModel;
}

/*virtual*/ int64_t CProductDefinition::calculateInstance(int64_t* piVertexBufferSize, int64_t* piIndexBufferSize) /*override*/
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

void CProductDefinition::CalculateMinMaxTransform(
	CProductInstance* pInstance,
	float fXTranslation, float fYTranslation, float fZTranslation,
	float& fXmin, float& fXmax,
	float& fYmin, float& fYmax,
	float& fZmin, float& fZmax)
{
	if (!hasGeometry())
	{
		return;
	}

	double _41 = pInstance->GetTransformationMatrix()->_41;
	double _42 = pInstance->GetTransformationMatrix()->_42;
	double _43 = pInstance->GetTransformationMatrix()->_43;

	pInstance->GetTransformationMatrix()->_41 += fXTranslation;
	pInstance->GetTransformationMatrix()->_42 += fYTranslation;
	pInstance->GetTransformationMatrix()->_43 += fZTranslation;

	CalculateMinMaxTransform(
		pInstance,
		fXmin, fXmax,
		fYmin, fYmax,
		fZmin, fZmax);

	pInstance->GetTransformationMatrix()->_41 = _41;
	pInstance->GetTransformationMatrix()->_42 = _42;
	pInstance->GetTransformationMatrix()->_43 = _43;
}

void CProductDefinition::CalculateMinMaxTransform(
	CProductInstance* pInstance,
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
					_transform(&vecPoint, pInstance->GetTransformationMatrix(), &vecPoint);
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
					_transform(&vecPoint, pInstance->GetTransformationMatrix(), &vecPoint);
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
					_transform(&vecPoint, pInstance->GetTransformationMatrix(), &vecPoint);
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
					_transform(&vecPoint, pInstance->GetTransformationMatrix(), &vecPoint);
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

void CProductDefinition::Scale(float fScaleFactor)
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
		m_vecInstances[iInstance]->Scale(fScaleFactor);
	}
}

int32_t CProductDefinition::GetNextInstance()
{
	if (++m_iNextInstance >= (int32_t)m_vecInstances.size())
	{
		m_iNextInstance = 0;
	}

	return m_iNextInstance;
}

void CProductDefinition::Calculate()
{
	// Format
	setSTEPFormatSettings();

	// Extra settings
	setSegmentation(GetModel(), 16, 0.);

	/* Geometry */

	ASSERT(m_pVertexBuffer == nullptr);
	m_pVertexBuffer = new _vertices_f(getVertexLength());

	ASSERT(m_pIndexBuffer == nullptr);
	m_pIndexBuffer = new _indices_i32();

	if (!calculate(m_pVertexBuffer, m_pIndexBuffer))
	{
		return;
	}

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
		ConceptualFace iConceptualFace = GetConceptualFace(
			m_iInstance,
			iConceptualFaceIndex,
			&iStartIndexTriangles, &iIndicesCountTriangles,
			&iStartIndexLines, &iIndicesCountLines,
			&iStartIndexPoints, &iIndicesCountPoints,
			0, 0,
			&iStartIndexConceptualFacePolygons, &iIndicesCountConceptualFacePolygons);

		/* Material */

		uint32_t iAmbientColor = CSTEPModel::DEFAULT_COLOR;
		uint32_t iDiffuseColor = CSTEPModel::DEFAULT_COLOR;
		uint32_t iEmissiveColor = CSTEPModel::DEFAULT_COLOR;
		uint32_t iSpecularColor = CSTEPModel::DEFAULT_COLOR;
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
	buildPointsCohorts(mapMaterial2ConcFacePoints, _oglUtils::getIndicesCountLimit());
}

