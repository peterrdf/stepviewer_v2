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
	SdaiModel iSdaiModel = sdaiGetInstanceModel(GetInstance());
	ASSERT(iSdaiModel != 0);

	OwlModel iOwlModel = 0;
	owlGetModel(iSdaiModel, &iOwlModel);
	ASSERT(iOwlModel != 0);

	return iOwlModel;
}

void CIFCInstance::Calculate()
{
	/*
	* Set up format
	*/
	uint64_t mask = 0;
	mask += flagbit2;        // PRECISION (32/64 bit)
	mask += flagbit3;        //	INDEX ARRAY (32/64 bit)
	mask += flagbit5;        // NORMALS
	mask += flagbit6;        // TEXTURE
	mask += flagbit8;        // TRIANGLES
	mask += flagbit9;        // LINES
	mask += flagbit10;       // POINTS
	mask += flagbit13;       // CONCEPTUAL FACE POLYGON

	uint64_t setting = 0;
	setting += 0;		     // SINGLE PRECISION (float)
	setting += 0;            // 32 BIT INDEX ARRAY (Int32)
	setting += flagbit5;     // NORMALS ON
	setting += 0;			 // TEXTURE OFF
	setting += flagbit8;     // TRIANGLES ON
	setting += flagbit9;     // LINES ON
	setting += flagbit10;    // POINTS ON
	setting += flagbit13;    // CONCEPTUAL FACE POLYGON ON
	SetFormat(getModel(), setting, mask);

	SdaiModel iSdaiModel = sdaiGetInstanceModel(GetInstance());
	ASSERT(iSdaiModel != 0);

	setFilter(iSdaiModel, flagbit1, flagbit1);

	setSegmentation(iSdaiModel, 16, 0.);

	ASSERT(m_pVertexBuffer == nullptr);
	m_pVertexBuffer = new _vertices_f(getVertexLength());

	ASSERT(m_pIndexBuffer == nullptr);
	m_pIndexBuffer = new _indices_i32();

	CalculateInstance(m_iInstance, &m_pVertexBuffer->size(), &m_pIndexBuffer->size(), nullptr);
	if ((m_pVertexBuffer->size() == 0) || (m_pIndexBuffer->size() == 0))
	{
		return;
	}

	/**
	* Retrieves the vertices
	*/
	m_pVertexBuffer->data() = new float[(uint32_t)m_pVertexBuffer->size() * m_pVertexBuffer->getVertexLength()];
	memset(m_pVertexBuffer->data(), 0, (uint32_t)m_pVertexBuffer->size() * m_pVertexBuffer->getVertexLength() * sizeof(float));

	UpdateInstanceVertexBuffer(m_iInstance, m_pVertexBuffer->data());

	/**
	* Retrieves the indices
	*/
	m_pIndexBuffer->data() = new int32_t[(uint32_t)m_pIndexBuffer->size()];
	memset(m_pIndexBuffer->data(), 0, (uint32_t)m_pIndexBuffer->size() * sizeof(int32_t));

	UpdateInstanceIndexBuffer(m_iInstance, m_pIndexBuffer->data());

	// MATERIAL : FACE INDEX, START INDEX, INIDCES COUNT, etc.
	MATERIALS mapMaterial2ConcFaces;
	MATERIALS mapMaterial2ConcFaceLines;
	MATERIALS mapMaterial2ConcFacePoints;

	//	http://rdf.bg/gkdoc/CP64/GetConceptualFaceCnt.html
	m_iConceptualFacesCount = GetConceptualFaceCnt(m_iInstance);
	for (int64_t iConceptualFaceIndex = 0; iConceptualFaceIndex < m_iConceptualFacesCount; iConceptualFaceIndex++)
	{
		//	http://rdf.bg/gkdoc/CP64/GetConceptualFaceEx.html
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

		uint32_t iAmbientColor = 0;
		uint32_t iDiffuseColor = 0;
		uint32_t iEmissiveColor = 0;
		uint32_t iSpecularColor = 0;
		float fTransparency = 1.f;

		OwlInstance iMaterialInstance = GetConceptualFaceMaterial(iConceptualFace);
		if (iMaterialInstance != 0)
		{
			int64_t* piInstances = nullptr;
			int64_t iCard = 0;
			GetObjectProperty(
				iMaterialInstance,
				GetPropertyByName(getModel(), "textures"),
				&piInstances,
				&iCard);

			iAmbientColor = GetMaterialColorAmbient(iMaterialInstance);
			iDiffuseColor = GetMaterialColorDiffuse(iMaterialInstance);
			iEmissiveColor = GetMaterialColorEmissive(iMaterialInstance);
			iSpecularColor = GetMaterialColorSpecular(iMaterialInstance);
			fTransparency = (float)COLOR_GET_W(iAmbientColor);
		}
		else
		{
			uint32_t iR = 10,
				iG = 150,
				iB = 10,
				iA = 255;
			uint32_t iDefaultColor = 256 * 256 * 256 * iR +
				256 * 256 * iG +
				256 * iB +
				iA;

			iAmbientColor = iDefaultColor;
			iDiffuseColor = iDefaultColor;
			iEmissiveColor = iDefaultColor;
			iSpecularColor = iDefaultColor;
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
			m_vecTriangles.push_back(_primitives(iStartIndexTriangles, iIndicesCountTriangles));

			auto itMaterial2ConceptualFaces = mapMaterial2ConcFaces.find(material);
			if (itMaterial2ConceptualFaces == mapMaterial2ConcFaces.end())
			{
				mapMaterial2ConcFaces[material] = vector<_face>{ _face(iConceptualFaceIndex, iStartIndexTriangles, iIndicesCountTriangles) };
			}
			else
			{
				itMaterial2ConceptualFaces->second.push_back(_face(iConceptualFaceIndex, iStartIndexTriangles, iIndicesCountTriangles));
			}
		}

		if (iIndicesCountConceptualFacePolygons > 0)
		{
			m_vecConcFacePolygons.push_back(_primitives(iStartIndexConceptualFacePolygons, iIndicesCountConceptualFacePolygons));
		}

		if (iIndicesCountLines > 0)
		{
			m_vecLines.push_back(_primitives(iStartIndexLines, iIndicesCountLines));

			auto itMaterial2ConcFaceLines = mapMaterial2ConcFaceLines.find(material);
			if (itMaterial2ConcFaceLines == mapMaterial2ConcFaceLines.end())
			{
				mapMaterial2ConcFaceLines[material] = vector<_face>{ _face(iConceptualFaceIndex, iStartIndexLines, iIndicesCountLines) };
			}
			else
			{
				itMaterial2ConcFaceLines->second.push_back(_face(iConceptualFaceIndex, iStartIndexLines, iIndicesCountLines));
			}
		}

		if (iIndicesCountPoints > 0)
		{
			m_vecPoints.push_back(_primitives(iStartIndexPoints, iIndicesCountPoints));

			auto itMaterial2ConcFacePoints = mapMaterial2ConcFacePoints.find(material);
			if (itMaterial2ConcFacePoints == mapMaterial2ConcFacePoints.end())
			{
				mapMaterial2ConcFacePoints[material] = vector<_face>{ _face(iConceptualFaceIndex, iStartIndexPoints, iIndicesCountPoints) };
			}
			else
			{
				itMaterial2ConcFacePoints->second.push_back(_face(iConceptualFaceIndex, iStartIndexPoints, iIndicesCountPoints));
			}
		} // if (iIndicesCountPoints > 0)
	} // for (int64_t iConceptualFaceIndex = ...	

	// Group the faces
	auto itMaterial2ConcFaces = mapMaterial2ConcFaces.begin();
	for (; itMaterial2ConcFaces != mapMaterial2ConcFaces.end(); itMaterial2ConcFaces++)
	{
		_cohortWithMaterial* pCohort = nullptr;

		for (size_t iConcFace = 0; iConcFace < itMaterial2ConcFaces->second.size(); iConcFace++)
		{
			_face& concFace = itMaterial2ConcFaces->second[iConcFace];

			int64_t iStartIndex = concFace.startIndex();
			int64_t iIndicesCount = concFace.indicesCount();

			/*
			* Split the conceptual face - isolated case
			*/
			if (iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				while (iIndicesCount > _oglUtils::getIndicesCountLimit())
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial2ConcFaces->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + _oglUtils::getIndicesCountLimit();
						iIndex++)
					{
						pNewCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
					}

					m_vecConcFacesCohorts.push_back(pNewCohort);

					/*
					* Update Conceptual face start index
					*/
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);

					iIndicesCount -= _oglUtils::getIndicesCountLimit();
					iStartIndex += _oglUtils::getIndicesCountLimit();
				}

				if (iIndicesCount > 0)
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial2ConcFaces->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + iIndicesCount;
						iIndex++)
					{
						pNewCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
					}

					m_vecConcFacesCohorts.push_back(pNewCohort);

					/*
					* Update Conceptual face start index
					*/
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);
				}

				continue;
			} // if (iIndicesCount > _oglUtils::GetIndicesCountLimit())	

			/*
			* Create material
			*/
			if (pCohort == nullptr)
			{
				pCohort = new _cohortWithMaterial(itMaterial2ConcFaces->first);

				m_vecConcFacesCohorts.push_back(pCohort);
			}

			/*
			* Check the limit
			*/
			if (pCohort->indices().size() + iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohortWithMaterial(itMaterial2ConcFaces->first);

				m_vecConcFacesCohorts.push_back(pCohort);
			}

			/*
			* Update Conceptual face start index
			*/
			concFace.startIndex() = pCohort->indices().size();

			/*
			* Add the indices
			*/
			for (int64_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
			}

			// Conceptual faces
			pCohort->faces().push_back(concFace);
		} // for (size_t iConcFace = ...
	} // for (; itMaterial2ConceptualFaces != ...

	// Group the polygons
	if (!m_vecConcFacePolygons.empty())
	{
		/*
		* Use the last cohort (if any)
		*/
		_cohort* pCohort = m_vecConcFacePolygonsCohorts.empty() ?
			nullptr : m_vecConcFacePolygonsCohorts[m_vecConcFacePolygonsCohorts.size() - 1];

		/*
		* Create the cohort
		*/
		if (pCohort == nullptr)
		{
			pCohort = new _cohort();
			m_vecConcFacePolygonsCohorts.push_back(pCohort);
		}

		for (size_t iFace = 0; iFace < m_vecConcFacePolygons.size(); iFace++)
		{
			int64_t iStartIndex = m_vecConcFacePolygons[iFace].startIndex();
			int64_t iIndicesCount = m_vecConcFacePolygons[iFace].indicesCount();

			/*
			* Split the conceptual face - isolated case
			*/
			if (iIndicesCount > _oglUtils::getIndicesCountLimit() / 2)
			{
				while (iIndicesCount > _oglUtils::getIndicesCountLimit() / 2)
				{
					pCohort = new _cohort();
					m_vecConcFacePolygonsCohorts.push_back(pCohort);

					int64_t iPreviousIndex = -1;
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + _oglUtils::getIndicesCountLimit() / 2;
						iIndex++)
					{
						if (m_pIndexBuffer->data()[iIndex] < 0)
						{
							iPreviousIndex = -1;

							continue;
						}

						if (iPreviousIndex != -1)
						{
							pCohort->indices().push_back(m_pIndexBuffer->data()[iPreviousIndex]);
							pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
						} // if (iPreviousIndex != -1)

						iPreviousIndex = iIndex;
					} // for (int_t iIndex = ...

					iIndicesCount -= _oglUtils::getIndicesCountLimit() / 2;
					iStartIndex += _oglUtils::getIndicesCountLimit() / 2;
				} // while (iIndicesCount > _oglUtils::GetIndicesCountLimit() / 2)

				if (iIndicesCount > 0)
				{
					pCohort = new _cohort();
					m_vecConcFacePolygonsCohorts.push_back(pCohort);

					int64_t iPreviousIndex = -1;
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + iIndicesCount;
						iIndex++)
					{
						if (m_pIndexBuffer->data()[iIndex] < 0)
						{
							iPreviousIndex = -1;

							continue;
						}

						if (iPreviousIndex != -1)
						{
							pCohort->indices().push_back(m_pIndexBuffer->data()[iPreviousIndex]);
							pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
						} // if (iPreviousIndex != -1)

						iPreviousIndex = iIndex;
					} // for (int_t iIndex = ...
				}

				continue;
			} // if (iIndicesCount > _oglUtils::GetIndicesCountLimit() / 2)

			/*
			* Check the limit
			*/
			if ((pCohort->indices().size() + (iIndicesCount * 2)) > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohort();
				m_vecConcFacePolygonsCohorts.push_back(pCohort);
			}

			int64_t iPreviousIndex = -1;
			for (int64_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				if (m_pIndexBuffer->data()[iIndex] < 0)
				{
					iPreviousIndex = -1;

					continue;
				}

				if (iPreviousIndex != -1)
				{
					pCohort->indices().push_back(m_pIndexBuffer->data()[iPreviousIndex]);
					pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
				} // if (iPreviousIndex != -1)

				iPreviousIndex = iIndex;
			} // for (int_t iIndex = ...
		} // for (size_t iFace = ...
	} // if (!m_vecConcFacePolygons.empty())

	// Group the lines
	auto itMaterial2ConcFaceLines = mapMaterial2ConcFaceLines.begin();
	for (; itMaterial2ConcFaceLines != mapMaterial2ConcFaceLines.end(); itMaterial2ConcFaceLines++)
	{
		_cohortWithMaterial* pCohort = nullptr;

		for (size_t iConcFace = 0; iConcFace < itMaterial2ConcFaceLines->second.size(); iConcFace++)
		{
			_face& concFace = itMaterial2ConcFaceLines->second[iConcFace];

			int64_t iStartIndex = concFace.startIndex();
			int64_t iIndicesCount = concFace.indicesCount();

			// Split the conceptual face - isolated case
			if (iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				while (iIndicesCount > _oglUtils::getIndicesCountLimit())
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial2ConcFaceLines->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + _oglUtils::getIndicesCountLimit();
						iIndex++)
					{
						pNewCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
					}

					linesCohorts().push_back(pNewCohort);

					// Update Conceptual face start index
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);

					iIndicesCount -= _oglUtils::getIndicesCountLimit();
					iStartIndex += _oglUtils::getIndicesCountLimit();
				}

				if (iIndicesCount > 0)
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial2ConcFaceLines->first);
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
			} // if (iIndicesCountTriangles > _oglUtils::GetIndicesCountLimit())	

			// Create material
			if (pCohort == nullptr)
			{
				pCohort = new _cohortWithMaterial(itMaterial2ConcFaceLines->first);

				linesCohorts().push_back(pCohort);
			}

			// Check the limit
			if (pCohort->indices().size() + iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohortWithMaterial(itMaterial2ConcFaceLines->first);

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
	} // for (; itMaterial2ConceptualFaces != ...

	// Group the points
	auto itMaterial2ConcFacePoints = mapMaterial2ConcFacePoints.begin();
	for (; itMaterial2ConcFacePoints != mapMaterial2ConcFacePoints.end(); itMaterial2ConcFacePoints++)
	{
		_cohortWithMaterial* pCohort = nullptr;

		for (size_t iConcFace = 0; iConcFace < itMaterial2ConcFacePoints->second.size(); iConcFace++)
		{
			_face& concFace = itMaterial2ConcFacePoints->second[iConcFace];

			int64_t iStartIndex = concFace.startIndex();
			int64_t iIndicesCount = concFace.indicesCount();

			/*
			* Split the conceptual face - isolated case
			*/
			if (iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				while (iIndicesCount > _oglUtils::getIndicesCountLimit())
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial2ConcFacePoints->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + _oglUtils::getIndicesCountLimit();
						iIndex++)
					{
						pNewCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
					}

					pointsCohorts().push_back(pNewCohort);

					/*
					* Update Conceptual face start index
					*/
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);

					iIndicesCount -= _oglUtils::getIndicesCountLimit();
					iStartIndex += _oglUtils::getIndicesCountLimit();
				}

				if (iIndicesCount > 0)
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial2ConcFacePoints->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + iIndicesCount;
						iIndex++)
					{
						pNewCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
					}

					pointsCohorts().push_back(pNewCohort);

					/*
					* Update Conceptual face start index
					*/
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);
				}

				continue;
			} // if (iIndicesCountTriangles > _oglUtils::GetIndicesCountLimit())	

			/*
			* Create material
			*/
			if (pCohort == nullptr)
			{
				pCohort = new _cohortWithMaterial(itMaterial2ConcFacePoints->first);

				pointsCohorts().push_back(pCohort);
			}

			/*
			* Check the limit
			*/
			if (pCohort->indices().size() + iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohortWithMaterial(itMaterial2ConcFacePoints->first);

				pointsCohorts().push_back(pCohort);
			}

			/*
			* Update Conceptual face start index
			*/
			concFace.startIndex() = pCohort->indices().size();

			/*
			* Add the indices
			*/
			for (int64_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
			}

			// Conceptual faces
			pCohort->faces().push_back(concFace);
		} // for (size_t iConcFace = ...
	} // for (; itMaterial2ConceptualFaces != ...
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

	/*
	* Triangles
	*/
	if (!m_vecTriangles.empty())
	{
		for (size_t iPrimitive = 0; iPrimitive < m_vecTriangles.size(); iPrimitive++)
		{
			for (int64_t iIndex = m_vecTriangles[iPrimitive].startIndex();
				iIndex < m_vecTriangles[iPrimitive].startIndex() + m_vecTriangles[iPrimitive].indicesCount();
				iIndex++)
			{
				fXmin = (float)fmin(fXmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength())]);
				fXmax = (float)fmax(fXmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength())]);
				fYmin = (float)fmin(fYmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 1]);
				fYmax = (float)fmax(fYmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 1]);
				fZmin = (float)fmin(fZmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 2]);
				fZmax = (float)fmax(fZmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPrimitive = ...
	} // if (!m_vecTriangles.empty())	

	/*
	* Conceptual faces polygons
	*/
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

				fXmin = (float)fmin(fXmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength())]);
				fXmax = (float)fmax(fXmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength())]);
				fYmin = (float)fmin(fYmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 1]);
				fYmax = (float)fmax(fYmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 1]);
				fZmin = (float)fmin(fZmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 2]);
				fZmax = (float)fmax(fZmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPrimitive = ...
	} // if (!m_vecConcFacePolygons.empty())

	/*
	* Lines
	*/
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

				fXmin = (float)fmin(fXmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength())]);
				fXmax = (float)fmax(fXmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength())]);
				fYmin = (float)fmin(fYmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 1]);
				fYmax = (float)fmax(fYmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 1]);
				fZmin = (float)fmin(fZmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 2]);
				fZmax = (float)fmax(fZmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPrimitive = ...
	} // if (!m_vecLines.empty())

	/*
	* Points
	*/
	if (!m_vecPoints.empty())
	{
		for (size_t iPrimitive = 0; iPrimitive < m_vecPoints.size(); iPrimitive++)
		{
			for (int64_t iIndex = m_vecPoints[iPrimitive].startIndex();
				iIndex < m_vecPoints[iPrimitive].startIndex() + m_vecPoints[iPrimitive].indicesCount();
				iIndex++)
			{
				fXmin = (float)fmin(fXmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength())]);
				fXmax = (float)fmax(fXmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength())]);
				fYmin = (float)fmin(fYmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 1]);
				fYmax = (float)fmax(fYmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 1]);
				fZmin = (float)fmin(fZmin, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 2]);
				fZmax = (float)fmax(fZmax, m_pVertexBuffer->data()[(m_pIndexBuffer->data()[iIndex] * getVertexLength()) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPrimitive = ...
	} // if (!m_vecPoints.empty())
}

void CIFCInstance::Scale(float fScaleFactor)
{
	if (!HasGeometry())
	{
		return;
	}

	// Vertices [-1.0 -> 1.0]
	for (int_t iVertex = 0; iVertex < m_pVertexBuffer->size(); iVertex++)
	{
		m_pVertexBuffer->data()[(iVertex * getVertexLength())] = m_pVertexBuffer->data()[(iVertex * getVertexLength())] / fScaleFactor;
		m_pVertexBuffer->data()[(iVertex * getVertexLength()) + 1] = m_pVertexBuffer->data()[(iVertex * getVertexLength()) + 1] / fScaleFactor;
		m_pVertexBuffer->data()[(iVertex * getVertexLength()) + 2] = m_pVertexBuffer->data()[(iVertex * getVertexLength()) + 2] / fScaleFactor;
	}
}
