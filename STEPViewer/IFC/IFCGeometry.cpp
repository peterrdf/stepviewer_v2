#include "stdafx.h"
#include "IFCGeometry.h"
#include "IFCModel.h"

#include <math.h>

// ************************************************************************************************
CIFCGeometry::CIFCGeometry(int64_t iID, SdaiInstance iSdaiInstance)
	: _ap_geometry(iSdaiInstance)
	, m_bReferenced(false)
{
	Calculate();
}

/*virtual*/ CIFCGeometry::~CIFCGeometry()
{}

void CIFCGeometry::Calculate()
{
	// Format
	setSTEPFormatSettings();

	// Extra settings
	const int_t flagbit1 = 2;
	setFilter(getSdaiModel(), flagbit1, flagbit1);
	setSegmentation(getSdaiModel(), 16, 0.);

	//
	// Geometry
	//

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

		//
		// Material
		//

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
