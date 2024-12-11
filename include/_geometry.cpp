#include "stdafx.h"
#include "_geometry.h"
#include "_oglUtils.h"

/*virtual*/ void _geometry::calculateCore()
{
	ASSERT(m_pVertexBuffer == nullptr);
	m_pVertexBuffer = new _vertices_f(getVertexLength());

	ASSERT(m_pIndexBuffer == nullptr);
	m_pIndexBuffer = new _indices_i32();

	if (!calculateInstance(m_pVertexBuffer, m_pIndexBuffer))
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

		//
		// Material
		// 

		uint32_t iAmbientColor = getDefaultColor();
		uint32_t iDiffuseColor = getDefaultColor();
		uint32_t iEmissiveColor = getDefaultColor();
		uint32_t iSpecularColor = getDefaultColor();
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

void _geometry::buildConcFacesCohorts(MATERIALS& mapMaterials, const GLsizei INDICES_COUNT_LIMIT)
{
	auto itMaterial = mapMaterials.begin();
	for (; itMaterial != mapMaterials.end(); itMaterial++)
	{
		_cohortWithMaterial* pCohort = nullptr;

		for (size_t iConcFace = 0; iConcFace < itMaterial->second.size(); iConcFace++)
		{
			_face& concFace = itMaterial->second[iConcFace];

			int64_t iStartIndex = concFace.startIndex();
			int64_t iIndicesCount = concFace.indicesCount();

			// Split the conceptual face - isolated case
			if (iIndicesCount > INDICES_COUNT_LIMIT)
			{
				while (iIndicesCount > INDICES_COUNT_LIMIT)
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + INDICES_COUNT_LIMIT;
						iIndex++)
					{
						pNewCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
					}

					concFacesCohorts().push_back(pNewCohort);

					// Update Conceptual face start index
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);

					iIndicesCount -= INDICES_COUNT_LIMIT;
					iStartIndex += INDICES_COUNT_LIMIT;
				}

				if (iIndicesCount > 0)
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + iIndicesCount;
						iIndex++)
					{
						pNewCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
					}

					concFacesCohorts().push_back(pNewCohort);

					// Update Conceptual face start index
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);
				}

				continue;
			} // if (iIndicesCount > INDICES_COUNT_LIMIT)	

			// Create cohort
			if (pCohort == nullptr)
			{
				pCohort = new _cohortWithMaterial(itMaterial->first);

				concFacesCohorts().push_back(pCohort);
			}

			// Check the limit
			if (pCohort->indices().size() + iIndicesCount > INDICES_COUNT_LIMIT)
			{
				pCohort = new _cohortWithMaterial(itMaterial->first);

				concFacesCohorts().push_back(pCohort);
			}

			// Update Conceptual face start index
			concFace.startIndex() = pCohort->indices().size();

			// Add the indices
			for (int64_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
			}

			// Conceptual faces
			pCohort->faces().push_back(concFace);
		} // for (size_t iConcFace = ...
	} // for (; itMaterial != ...
}

void _geometry::buildFacePolygonsCohorts(const GLsizei INDICES_COUNT_LIMIT)
{
	if (m_vecFacePolygons.empty())
	{
		return;
	}

	// Use the last cohort (if any)
	_cohort* pCohort = facePolygonsCohorts().empty() ? nullptr : facePolygonsCohorts()[facePolygonsCohorts().size() - 1];

	// Create the cohort
	if (pCohort == nullptr)
	{
		pCohort = new _cohort();
		facePolygonsCohorts().push_back(pCohort);
	}

	for (size_t iFace = 0; iFace < m_vecFacePolygons.size(); iFace++)
	{
		int64_t iStartIndex = m_vecFacePolygons[iFace].startIndex();
		int64_t iIndicesCount = m_vecFacePolygons[iFace].indicesCount();

		// Split the conceptual face - isolated case
		if (iIndicesCount > INDICES_COUNT_LIMIT / 2)
		{
			while (iIndicesCount > INDICES_COUNT_LIMIT / 2)
			{
				pCohort = new _cohort();
				facePolygonsCohorts().push_back(pCohort);

				for (int64_t iIndex = iStartIndex;
					iIndex < iStartIndex + INDICES_COUNT_LIMIT / 2;
					iIndex += 2)
				{
					pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex + 0]);
					pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex + 1]);
				}

				iIndicesCount -= INDICES_COUNT_LIMIT / 2;
				iStartIndex += INDICES_COUNT_LIMIT / 2;
			} // while (iIndicesCount > INDICES_COUNT_LIMIT / 2)

			if (iIndicesCount > 0)
			{
				pCohort = new _cohort();
				facePolygonsCohorts().push_back(pCohort);

				for (int64_t iIndex = iStartIndex;
					iIndex < iStartIndex + iIndicesCount;
					iIndex += 2)
				{
					pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex + 0]);
					pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex + 1]);
				}
			}

			continue;
		} // if (iIndicesCount > INDICES_COUNT_LIMIT / 2)

		// Check the limit
		if ((pCohort->indices().size() + (iIndicesCount * 2)) > INDICES_COUNT_LIMIT)
		{
			pCohort = new _cohort();
			facePolygonsCohorts().push_back(pCohort);
		}

		for (int64_t iIndex = iStartIndex;
			iIndex < iStartIndex + iIndicesCount;
			iIndex += 2)
		{
			pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex + 0]);
			pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex + 1]);
		}
	} // for (size_t iFace = ...
}

void _geometry::buildConcFacePolygonsCohorts(const GLsizei INDICES_COUNT_LIMIT)
{
	if (m_vecConcFacePolygons.empty())
	{
		return;
	}

	// Use the last cohort (if any)
	_cohort* pCohort = concFacePolygonsCohorts().empty() ? nullptr : concFacePolygonsCohorts()[concFacePolygonsCohorts().size() - 1];

	// Create the cohort
	if (pCohort == nullptr)
	{
		pCohort = new _cohort();
		concFacePolygonsCohorts().push_back(pCohort);
	}

	for (size_t iFace = 0; iFace < m_vecConcFacePolygons.size(); iFace++)
	{
		int64_t iStartIndex = m_vecConcFacePolygons[iFace].startIndex();
		int64_t iIndicesCount = m_vecConcFacePolygons[iFace].indicesCount();

		// Split the conceptual face - isolated case
		if (iIndicesCount > INDICES_COUNT_LIMIT / 2)
		{
			while (iIndicesCount > INDICES_COUNT_LIMIT / 2)
			{
				pCohort = new _cohort();
				concFacePolygonsCohorts().push_back(pCohort);

				for (int64_t iIndex = iStartIndex;
					iIndex < iStartIndex + INDICES_COUNT_LIMIT / 2;
					iIndex += 2)
				{
					pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex + 0]);
					pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex + 1]);
				}

				iIndicesCount -= INDICES_COUNT_LIMIT / 2;
				iStartIndex += INDICES_COUNT_LIMIT / 2;
			} // while (iIndicesCount > INDICES_COUNT_LIMIT / 2)

			if (iIndicesCount > 0)
			{
				pCohort = new _cohort();
				concFacePolygonsCohorts().push_back(pCohort);

				for (int64_t iIndex = iStartIndex;
					iIndex < iStartIndex + iIndicesCount;
					iIndex += 2)
				{
					pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex + 0]);
					pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex + 1]);
				}
			}

			continue;
		} // if (iIndicesCount > INDICES_COUNT_LIMIT / 2)

		// Check the limit
		if ((pCohort->indices().size() + (iIndicesCount * 2)) > INDICES_COUNT_LIMIT)
		{
			pCohort = new _cohort();
			concFacePolygonsCohorts().push_back(pCohort);
		}

		for (int64_t iIndex = iStartIndex;
			iIndex < iStartIndex + iIndicesCount;
			iIndex += 2)
		{
			pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex + 0]);
			pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex + 1]);
		}
	} // for (size_t iFace = ...
}

void _geometry::buildLinesCohorts(MATERIALS& mapMaterials, const GLsizei INDICES_COUNT_LIMIT)
{
	auto itMaterial = mapMaterials.begin();
	for (; itMaterial != mapMaterials.end(); itMaterial++)
	{
		_cohortWithMaterial* pCohort = nullptr;

		for (size_t iConcFace = 0; iConcFace < itMaterial->second.size(); iConcFace++)
		{
			_face& concFace = itMaterial->second[iConcFace];

			int64_t iStartIndex = concFace.startIndex();
			int64_t iIndicesCount = concFace.indicesCount();

			// Split the conceptual face - isolated case
			if (iIndicesCount > INDICES_COUNT_LIMIT)
			{
				while (iIndicesCount > INDICES_COUNT_LIMIT)
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + INDICES_COUNT_LIMIT;
						iIndex++)
					{
						pNewCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
					}

					linesCohorts().push_back(pNewCohort);

					// Update Conceptual face start index
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);

					iIndicesCount -= INDICES_COUNT_LIMIT;
					iStartIndex += INDICES_COUNT_LIMIT;
				}

				if (iIndicesCount > 0)
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + iIndicesCount;
						iIndex++)
					{
						pNewCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
					}

					linesCohorts().push_back(pNewCohort);

					// Update Conceptual face start index
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);
				}

				continue;
			} // if (iIndicesCountTriangles > INDICES_COUNT_LIMIT)	

			// Create cohort
			if (pCohort == nullptr)
			{
				pCohort = new _cohortWithMaterial(itMaterial->first);

				linesCohorts().push_back(pCohort);
			}

			// Check the limit
			if (pCohort->indices().size() + iIndicesCount > INDICES_COUNT_LIMIT)
			{
				pCohort = new _cohortWithMaterial(itMaterial->first);

				linesCohorts().push_back(pCohort);
			}

			// Update Conceptual face start index
			concFace.startIndex() = pCohort->indices().size();

			// Add the indices
			for (int64_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
			}

			// Conceptual faces
			pCohort->faces().push_back(concFace);
		} // for (size_t iConcFace = ...
	} // for (; itMaterial != ...
}

void _geometry::buildPointsCohorts(MATERIALS& mapMaterials, const GLsizei INDICES_COUNT_LIMIT)
{
	auto itMaterial = mapMaterials.begin();
	for (; itMaterial != mapMaterials.end(); itMaterial++)
	{
		_cohortWithMaterial* pCohort = nullptr;

		for (size_t iConcFace = 0; iConcFace < itMaterial->second.size(); iConcFace++)
		{
			_face& concFace = itMaterial->second[iConcFace];

			int64_t iStartIndex = concFace.startIndex();
			int64_t iIndicesCount = concFace.indicesCount();

			// Split the conceptual face - isolated case
			if (iIndicesCount > INDICES_COUNT_LIMIT)
			{
				while (iIndicesCount > INDICES_COUNT_LIMIT)
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + INDICES_COUNT_LIMIT;
						iIndex++)
					{
						pNewCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
					}

					pointsCohorts().push_back(pNewCohort);

					// Update Conceptual face start index
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);

					iIndicesCount -= INDICES_COUNT_LIMIT;
					iStartIndex += INDICES_COUNT_LIMIT;
				}

				if (iIndicesCount > 0)
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + iIndicesCount;
						iIndex++)
					{
						pNewCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
					}

					pointsCohorts().push_back(pNewCohort);

					// Update Conceptual face start index
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);
				}

				continue;
			} // if (iIndicesCountTriangles > INDICES_COUNT_LIMIT)	

			// Create cohort
			if (pCohort == nullptr)
			{
				pCohort = new _cohortWithMaterial(itMaterial->first);

				pointsCohorts().push_back(pCohort);
			}

			// Check the limit
			if (pCohort->indices().size() + iIndicesCount > INDICES_COUNT_LIMIT)
			{
				pCohort = new _cohortWithMaterial(itMaterial->first);

				pointsCohorts().push_back(pCohort);
			}

			// Update Conceptual face start index
			concFace.startIndex() = pCohort->indices().size();

			// Add the indices
			for (int64_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
			}

			// Add Conceptual faces
			pCohort->faces().push_back(concFace);
		} // for (size_t iConcFace = ...
	} // for (; itMaterial != ...
}