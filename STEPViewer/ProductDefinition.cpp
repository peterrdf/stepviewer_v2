#include "stdafx.h"
#include "ProductDefinition.h"
#include "Generic.h"

// ------------------------------------------------------------------------------------------------
CProductDefinition::CProductDefinition()
	: m_iExpressID(0)
	, m_iInstance(0)
	, m_vecProductInstances()
	, m_iNextProductInstance(-1)
	, m_strId(L"")
	, m_strName(L"")
	, m_strDescription(L"")
	, m_strProductId(L"")
	, m_strProductName(L"")
	, m_iRelatingProductRefs(0)
	, m_iRelatedProductRefs(0)
	, m_bCalculated(false)
	, m_pVertexBuffer(NULL)
	, m_iVerticesCount(0)
	, m_pIndexBuffer(NULL)
	, m_iIndicesCount(0)
	, m_iConceptualFacesCount(0)
	, m_vecTriangles()
	, m_vecLines()
	, m_vecPoints()
	, m_vecConceptualFacesPolygons()
	, m_vecMaterials()
	, m_vecLinesCohorts()
	, m_vecPointsCohorts()
	, m_vecConceptualFacesCohorts()
	, m_bEnable(true)
	, m_iVBO(0)
	, m_iVBOOffset(0)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CProductDefinition::~CProductDefinition()
{
	Clean();
}

// ------------------------------------------------------------------------------------------------
void CProductDefinition::Calculate()
{
	if (m_bCalculated)
	{
		return;
	}

	m_bCalculated = true;

	int64_t iModel = GetModel(m_iInstance);

	/*
	* Set up format
	*/
	int_t setting = 0, mask = 0;
	mask += flagbit2;        // PRECISION (32/64 bit)
	mask += flagbit3;        //	INDEX ARRAY (32/64 bit)
	mask += flagbit5;        // NORMALS
	mask += flagbit6;        // TEXTURE
	mask += flagbit8;        // TRIANGLES
	mask += flagbit9;        // LINES
	mask += flagbit10;       // POINTS
	mask += flagbit12;       // WIREFRAME
	mask += flagbit13;       // CONCEPTUAL FACE POLYGON
	mask += flagbit24;		 //	AMBIENT
	mask += flagbit25;		 //	DIFFUSE
	mask += flagbit26;		 //	EMISSIVE
	mask += flagbit27;		 //	SPECULAR

	setting += 0;		     // SINGLE PRECISION (float)
	setting += 0;            // 32 BIT INDEX ARRAY (Int32)
	setting += flagbit5;     // NORMALS ON
	setting += 0;			 // TEXTURE OFF
	setting += flagbit8;     // TRIANGLES ON
	setting += flagbit9;     // LINES ON
	setting += flagbit10;    // POINTS ON
	setting += flagbit12;    // WIREFRAME ON
	setting += flagbit13;    // CONCEPTUAL FACE POLYGON ON
	setting += flagbit24;	 //	AMBIENT
	setting += flagbit25;	 //	DIFFUSE
	setting += flagbit26;	 //	EMISSIVE
	setting += flagbit27;	 //	SPECULAR

	//	http://rdf.bg/gkdoc/CP64/SetFormat.html
	int64_t iVertexElementSizeInBytes = SetFormat(iModel, setting, mask);
	ASSERT(iVertexElementSizeInBytes == VERTEX_LENGTH * sizeof(float));

	CalculateInstance(m_iInstance, &m_iVerticesCount, &m_iIndicesCount, nullptr);

	if ((m_iVerticesCount == 0) || (m_iIndicesCount == 0))
	{
		return;
	}

	m_pVertexBuffer = new float[m_iVerticesCount * ((int_t)iVertexElementSizeInBytes / sizeof(float))];
	m_pIndexBuffer = new int32_t[m_iIndicesCount];

	//	http://rdf.bg/gkdoc/CP64/UpdateInstanceVertexBuffer.html
	UpdateInstanceVertexBuffer(m_iInstance, m_pVertexBuffer);

	//	http://rdf.bg/gkdoc/CP64/UpdateInstanceIndexBuffer.html
	UpdateInstanceIndexBuffer(m_iInstance, m_pIndexBuffer);

	// MATERIAL : FACE INDEX, START INDEX, INIDCES COUNT, etc.
	MATERIALS mapMaterial2ConceptualFaces;

	//	http://rdf.bg/gkdoc/CP64/GetConceptualFaceCnt.html
	m_iConceptualFacesCount = GetConceptualFaceCnt(m_iInstance);
	for (int64_t iConceptualFace = 0; iConceptualFace < m_iConceptualFacesCount; iConceptualFace++)
	{
		//	http://rdf.bg/gkdoc/CP64/GetConceptualFaceEx.html
		int64_t iStartIndexTriangles = 0;
		int64_t iIndicesCountTriangles = 0;
		int64_t iStartIndexLines = 0;
		int64_t iLinesIndicesCount = 0;
		int64_t iStartIndexPoints = 0;
		int64_t iPointsIndicesCount = 0;
		int64_t iStartIndexConceptualFacePolygons = 0;
		int64_t iConceptualFacePolygonsIndicesCount = 0;
		GetConceptualFaceEx(m_iInstance, iConceptualFace,
			&iStartIndexTriangles, &iIndicesCountTriangles,
			&iStartIndexLines, &iLinesIndicesCount,
			&iStartIndexPoints, &iPointsIndicesCount,
			0, 0,
			&iStartIndexConceptualFacePolygons, &iConceptualFacePolygonsIndicesCount);

		if (iIndicesCountTriangles > 0)
		{
			/*
			* Material
			*/
			int32_t iIndexValue = *(m_pIndexBuffer + iStartIndexTriangles);
			iIndexValue *= VERTEX_LENGTH;

			float fColor = *(m_pVertexBuffer + iIndexValue + 6);
			unsigned int iAmbientColor = *(reinterpret_cast<unsigned int*>(&fColor));
			float fTransparency = (float)(iAmbientColor & (255)) / (float)255;

			fColor = *(m_pVertexBuffer + iIndexValue + 7);
			unsigned int iDiffuseColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(m_pVertexBuffer + iIndexValue + 8);
			unsigned int iEmissiveColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(m_pVertexBuffer + iIndexValue + 9);
			unsigned int iSpecularColor = *(reinterpret_cast<unsigned int*>(&fColor));

			/*
			* Material
			*/
			CSTEPGeometryWithMaterial* pGeometryWithMaterial = new CSTEPGeometryWithMaterial(
				iAmbientColor, iDiffuseColor, iEmissiveColor, iSpecularColor, fTransparency,
				NULL);

			MATERIALS::iterator itMaterial2ConceptualFaces = mapMaterial2ConceptualFaces.find(*pGeometryWithMaterial);
			if (itMaterial2ConceptualFaces == mapMaterial2ConceptualFaces.end())
			{
				vector<CConceptualFace> vecConceptualFaces;

				CConceptualFace conceptualFace;
				conceptualFace.index() = iConceptualFace;
				conceptualFace.trianglesStartIndex() = iStartIndexTriangles;
				conceptualFace.trianglesIndicesCount() = iIndicesCountTriangles;

				vecConceptualFaces.push_back(conceptualFace);

				mapMaterial2ConceptualFaces[*pGeometryWithMaterial] = vecConceptualFaces;
			}
			else
			{
				CConceptualFace conceptualFace;
				conceptualFace.index() = iConceptualFace;
				conceptualFace.trianglesStartIndex() = iStartIndexTriangles;
				conceptualFace.trianglesIndicesCount() = iIndicesCountTriangles;

				itMaterial2ConceptualFaces->second.push_back(conceptualFace);
			}

			delete pGeometryWithMaterial;
		}

		if (iIndicesCountTriangles > 0)
		{
			m_vecTriangles.push_back(pair<int64_t, int64_t>(iStartIndexTriangles, iIndicesCountTriangles));
		}

		if (iLinesIndicesCount > 0)
		{
			m_vecLines.push_back(pair<int64_t, int64_t>(iStartIndexLines, iLinesIndicesCount));
		}

		if (iPointsIndicesCount > 0)
		{
			m_vecPoints.push_back(pair<int64_t, int64_t>(iStartIndexPoints, iPointsIndicesCount));
		}

		if (iConceptualFacePolygonsIndicesCount > 0)
		{
			m_vecConceptualFacesPolygons.push_back(pair<int64_t, int64_t>(iStartIndexConceptualFacePolygons, iConceptualFacePolygonsIndicesCount));
		}
	} // for (int64_t iConceptualFace = ...	

		/*
	* Group the triangles
	*/
	MATERIALS::iterator itMaterial2ConceptualFaces = mapMaterial2ConceptualFaces.begin();
	for (; itMaterial2ConceptualFaces != mapMaterial2ConceptualFaces.end(); itMaterial2ConceptualFaces++)
	{
		CSTEPGeometryWithMaterial* pGeometryWithMaterial = NULL;

		for (size_t iConceptualFace = 0; iConceptualFace < itMaterial2ConceptualFaces->second.size(); iConceptualFace++)
		{
			CConceptualFace& conceptualFace = itMaterial2ConceptualFaces->second[iConceptualFace];

			int_t iStartIndexTriangles = conceptualFace.trianglesStartIndex();
			int_t iIndicesCountTriangles = conceptualFace.trianglesIndicesCount();

			/*
			* Split the conceptual face - isolated case
			*/
			if (iIndicesCountTriangles > MAX_INDICES_COUNT)
			{
				while (iIndicesCountTriangles > MAX_INDICES_COUNT)
				{
					// INDICES
					CSTEPGeometryWithMaterial* pNewMaterial = new CSTEPGeometryWithMaterial(itMaterial2ConceptualFaces->first);
					for (int_t iIndex = iStartIndexTriangles; iIndex < iStartIndexTriangles + MAX_INDICES_COUNT; iIndex++)
					{
						pNewMaterial->addIndex(m_pIndexBuffer[iIndex]);
					}

					m_vecMaterials.push_back(pNewMaterial);

					/*
					* Update Conceptual face start index
					*/
					conceptualFace.trianglesStartIndex() = 0;

					// Conceptual faces
					pNewMaterial->conceptualFaces().push_back(conceptualFace);

					iIndicesCountTriangles -= MAX_INDICES_COUNT;
					iStartIndexTriangles += MAX_INDICES_COUNT;
				}

				if (iIndicesCountTriangles > 0)
				{
					// INDICES
					CSTEPGeometryWithMaterial* pNewMaterial = new CSTEPGeometryWithMaterial(itMaterial2ConceptualFaces->first);
					for (int_t iIndex = iStartIndexTriangles; iIndex < iStartIndexTriangles + iIndicesCountTriangles; iIndex++)
					{
						pNewMaterial->addIndex(m_pIndexBuffer[iIndex]);
					}

					m_vecMaterials.push_back(pNewMaterial);

					/*
					* Update Conceptual face start index
					*/
					conceptualFace.trianglesStartIndex() = 0;

					// Conceptual faces
					pNewMaterial->conceptualFaces().push_back(conceptualFace);
				}

				continue;
			} // if (iIndicesCountTriangles > MAX_INDICES_COUNT)	

			/*
			* Create material
			*/
			if (pGeometryWithMaterial == NULL)
			{
				pGeometryWithMaterial = new CSTEPGeometryWithMaterial(itMaterial2ConceptualFaces->first);

				m_vecMaterials.push_back(pGeometryWithMaterial);
			}

			/*
			* Check the limit
			*/
			if (pGeometryWithMaterial->getIndicesCount() + iIndicesCountTriangles > MAX_INDICES_COUNT)
			{
				pGeometryWithMaterial = new CSTEPGeometryWithMaterial(itMaterial2ConceptualFaces->first);

				m_vecMaterials.push_back(pGeometryWithMaterial);
			}

			/*
			* Update Conceptual face start index
			*/
			conceptualFace.trianglesStartIndex() = pGeometryWithMaterial->getIndicesCount();

			/*
			* Add the indices
			*/
			for (int_t iIndex = iStartIndexTriangles; iIndex < iStartIndexTriangles + iIndicesCountTriangles; iIndex++)
			{
				pGeometryWithMaterial->addIndex(m_pIndexBuffer[iIndex]);
			}

			// Conceptual faces
			pGeometryWithMaterial->conceptualFaces().push_back(conceptualFace);
		} // for (size_t iConceptualFace = ...				
	} // for (; itMaterial2ConceptualFaces != ...

	/*
	* Group the polygons
	*/
	if (!m_vecConceptualFacesPolygons.empty())
	{
		/*
		* Use the last cohort (if any)
		*/
		CWireframesCohort* pWireframesCohort = m_vecConceptualFacesCohorts.empty() ? NULL : m_vecConceptualFacesCohorts[m_vecConceptualFacesCohorts.size() - 1];

		/*
		* Create the cohort
		*/
		if (pWireframesCohort == NULL)
		{
			pWireframesCohort = new CWireframesCohort();
			m_vecConceptualFacesCohorts.push_back(pWireframesCohort);
		}

		for (size_t iFace = 0; iFace < m_vecConceptualFacesPolygons.size(); iFace++)
		{
			int_t iStartIndexFacesPolygons = m_vecConceptualFacesPolygons[iFace].first;
			int_t iIndicesFacesPolygonsCount = m_vecConceptualFacesPolygons[iFace].second;

			/*
			* Split the conceptual face - isolated case
			*/
			if (iIndicesFacesPolygonsCount > MAX_INDICES_COUNT / 2)
			{
				while (iIndicesFacesPolygonsCount > MAX_INDICES_COUNT / 2)
				{
					pWireframesCohort = new CWireframesCohort();
					m_vecConceptualFacesCohorts.push_back(pWireframesCohort);

					int_t iPreviousIndex = -1;
					for (int_t iIndex = iStartIndexFacesPolygons; iIndex < iStartIndexFacesPolygons + MAX_INDICES_COUNT / 2; iIndex++)
					{
						if (m_pIndexBuffer[iIndex] < 0)
						{
							iPreviousIndex = -1;

							continue;
						}

						if (iPreviousIndex != -1)
						{
							pWireframesCohort->addIndex(m_pIndexBuffer[iPreviousIndex]);
							pWireframesCohort->addIndex(m_pIndexBuffer[iIndex]);
						} // if (iPreviousIndex != -1)

						iPreviousIndex = iIndex;
					} // for (int_t iIndex = ...

					iIndicesFacesPolygonsCount -= MAX_INDICES_COUNT / 2;
					iStartIndexFacesPolygons += MAX_INDICES_COUNT / 2;
				} // while (iIndicesFacesPolygonsCount > MAX_INDICES_COUNT / 2)

				if (iIndicesFacesPolygonsCount > 0)
				{
					pWireframesCohort = new CWireframesCohort();
					m_vecConceptualFacesCohorts.push_back(pWireframesCohort);

					int_t iPreviousIndex = -1;
					for (int_t iIndex = iStartIndexFacesPolygons; iIndex < iStartIndexFacesPolygons + iIndicesFacesPolygonsCount; iIndex++)
					{
						if (m_pIndexBuffer[iIndex] < 0)
						{
							iPreviousIndex = -1;

							continue;
						}

						if (iPreviousIndex != -1)
						{
							pWireframesCohort->addIndex(m_pIndexBuffer[iPreviousIndex]);
							pWireframesCohort->addIndex(m_pIndexBuffer[iIndex]);
						} // if (iPreviousIndex != -1)

						iPreviousIndex = iIndex;
					} // for (int_t iIndex = ...
				}

				continue;
			} // if (iIndicesFacesPolygonsCount > MAX_INDICES_COUNT / 2)

			/*
			* Check the limit
			*/
			if ((pWireframesCohort->getIndicesCount() + (iIndicesFacesPolygonsCount * 2)) > MAX_INDICES_COUNT)
			{
				pWireframesCohort = new CWireframesCohort();
				m_vecConceptualFacesCohorts.push_back(pWireframesCohort);
			}

			int_t iPreviousIndex = -1;
			for (int_t iIndex = iStartIndexFacesPolygons; iIndex < iStartIndexFacesPolygons + iIndicesFacesPolygonsCount; iIndex++)
			{
				if (m_pIndexBuffer[iIndex] < 0)
				{
					iPreviousIndex = -1;

					continue;
				}

				if (iPreviousIndex != -1)
				{
					pWireframesCohort->addIndex(m_pIndexBuffer[iPreviousIndex]);
					pWireframesCohort->addIndex(m_pIndexBuffer[iIndex]);
				} // if (iPreviousIndex != -1)

				iPreviousIndex = iIndex;
			} // for (int_t iIndex = ...
		} // for (size_t iFace = ...

#ifdef _DEBUG
		for (size_t iCohort = 0; iCohort < m_vecConceptualFacesCohorts.size(); iCohort++)
		{
			ASSERT(m_vecConceptualFacesCohorts[iCohort]->getIndicesCount() <= MAX_INDICES_COUNT);
		}
#endif
	} // if (!m_vecConceptualFacesPolygons.empty())	

	/*
	* Group the lines
	*/
	if (!m_vecLines.empty())
	{
		/*
		* Use the last cohort (if any)
		*/
		CLinesCohort* pLinesCohort = m_vecLinesCohorts.empty() ? NULL : m_vecLinesCohorts[m_vecLinesCohorts.size() - 1];

		/*
		* Create the cohort
		*/
		if (pLinesCohort == NULL)
		{
			pLinesCohort = new CLinesCohort();
			m_vecLinesCohorts.push_back(pLinesCohort);
		}

		for (size_t iFace = 0; iFace < m_vecLines.size(); iFace++)
		{
			int_t iStartIndexLines = m_vecLines[iFace].first;
			int_t iIndicesLinesCount = m_vecLines[iFace].second;

			/*
			* Check the limit
			*/
			if (pLinesCohort->getIndicesCount() + iIndicesLinesCount > MAX_INDICES_COUNT)
			{
				pLinesCohort = new CLinesCohort();
				m_vecLinesCohorts.push_back(pLinesCohort);
			}

			for (int_t iIndex = iStartIndexLines; iIndex < iStartIndexLines + iIndicesLinesCount; iIndex++)
			{
				if (m_pIndexBuffer[iIndex] < 0)
				{
					continue;
				}

				pLinesCohort->addIndex(m_pIndexBuffer[iIndex]);
			} // for (int_t iIndex = ...
		} // for (size_t iFace = ...

#ifdef _DEBUG
		for (size_t iCohort = 0; iCohort < m_vecLinesCohorts.size(); iCohort++)
		{
			ASSERT(m_vecLinesCohorts[iCohort]->getIndicesCount() <= MAX_INDICES_COUNT);
		}
#endif
	} // if (!m_vecLines.empty())		

	/*
	* Group the points
	*/
	if (!m_vecPoints.empty())
	{
		/*
		* Use the last cohort (if any)
		*/
		CPointsCohort* pPointsCohort = m_vecPointsCohorts.empty() ? NULL : m_vecPointsCohorts[m_vecPointsCohorts.size() - 1];

		/*
		* Create the cohort
		*/
		if (pPointsCohort == NULL)
		{
			pPointsCohort = new CPointsCohort();
			m_vecPointsCohorts.push_back(pPointsCohort);
		}

		for (size_t iFace = 0; iFace < m_vecPoints.size(); iFace++)
		{
			int_t iStartIndexPoints = m_vecPoints[iFace].first;
			int_t iIndicesPointsCount = m_vecPoints[iFace].second;

			/*
			* Check the limit
			*/
			if (pPointsCohort->getIndicesCount() + iIndicesPointsCount > MAX_INDICES_COUNT)
			{
				pPointsCohort = new CPointsCohort();
				m_vecPointsCohorts.push_back(pPointsCohort);
			}

			for (int_t iIndex = iStartIndexPoints; iIndex < iStartIndexPoints + iIndicesPointsCount; iIndex++)
			{
				pPointsCohort->addIndex(m_pIndexBuffer[iIndex]);
			}
		} // for (size_t iFace = ...

#ifdef _DEBUG
		for (size_t iCohort = 0; iCohort < m_vecPointsCohorts.size(); iCohort++)
		{
			ASSERT(m_vecPointsCohorts[iCohort]->getIndicesCount() <= MAX_INDICES_COUNT);
		}
#endif
	} // if (!m_vecPoints.empty())
}

// ------------------------------------------------------------------------------------------------
void CProductDefinition::CalculateMinMax(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax)
{
	if (m_iVerticesCount == 0)
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
			for (int64_t iIndex = m_vecTriangles[iTriangle].first; iIndex < m_vecTriangles[iTriangle].first + m_vecTriangles[iTriangle].second; iIndex++)
			{
				fXmin = (float)fmin(fXmin, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH)]);
				fXmax = (float)fmax(fXmax, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH)]);
				fYmin = (float)fmin(fYmin, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iTriangle = ...
	} // if (!m_vecTriangles.empty())	

	/*
	* Conceptual faces polygons
	*/
	if (!m_vecConceptualFacesPolygons.empty())
	{
		for (size_t iPolygon = 0; iPolygon < m_vecConceptualFacesPolygons.size(); iPolygon++)
		{
			for (int64_t iIndex = m_vecConceptualFacesPolygons[iPolygon].first; iIndex < m_vecConceptualFacesPolygons[iPolygon].first + m_vecConceptualFacesPolygons[iPolygon].second; iIndex++)
			{
				if ((getIndices()[iIndex] == -1) || (getIndices()[iIndex] == -2))
				{
					continue;
				}

				fXmin = (float)fmin(fXmin, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH)]);
				fXmax = (float)fmax(fXmax, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH)]);
				fYmin = (float)fmin(fYmin, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPolygon = ...
	} // if (!m_vecConceptualFacesPolygons.empty())

	/*
	* Lines
	*/
	if (!m_vecLines.empty())
	{
		for (size_t iPolygon = 0; iPolygon < m_vecLines.size(); iPolygon++)
		{
			for (int64_t iIndex = m_vecLines[iPolygon].first; iIndex < m_vecLines[iPolygon].first + m_vecLines[iPolygon].second; iIndex++)
			{
				if (getIndices()[iIndex] == -1)
				{
					continue;
				}

				fXmin = (float)fmin(fXmin, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH)]);
				fXmax = (float)fmax(fXmax, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH)]);
				fYmin = (float)fmin(fYmin, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 2]);
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
			for (int64_t iIndex = m_vecPoints[iPolygon].first; iIndex < m_vecPoints[iPolygon].first + m_vecPoints[iPolygon].second; iIndex++)
			{
				fXmin = (float)fmin(fXmin, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH)]);
				fXmax = (float)fmax(fXmax, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH)]);
				fYmin = (float)fmin(fYmin, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPolygon = ...
	} // if (!m_vecPoints.empty())
}

// ------------------------------------------------------------------------------------------------
void CProductDefinition::CalculateMinMaxTransform(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax)
{
	if (m_iVerticesCount == 0)
	{
		return;
	}

	for (size_t iInstance = 0; iInstance < m_vecProductInstances.size(); iInstance++)
	{
		auto pProductInstance = m_vecProductInstances[iInstance];	

		pProductInstance->m_fXmin = FLT_MAX;
		pProductInstance->m_fXmax = -FLT_MAX;
		pProductInstance->m_fYmin = FLT_MAX;
		pProductInstance->m_fYmax = -FLT_MAX;
		pProductInstance->m_fZmin = FLT_MAX;
		pProductInstance->m_fZmax = -FLT_MAX;

		CalculateMinMaxTransform(
			pProductInstance, 
			pProductInstance->m_fXmin, pProductInstance->m_fXmax, 
			pProductInstance->m_fYmin, pProductInstance->m_fYmax, 
			pProductInstance->m_fZmin, pProductInstance->m_fZmax);

		fXmin = (float)fmin(fXmin, pProductInstance->m_fXmin);
		fXmax = (float)fmax(fXmax, pProductInstance->m_fXmax);
		fYmin = (float)fmin(fYmin, pProductInstance->m_fYmin);
		fYmax = (float)fmax(fYmax, pProductInstance->m_fYmax);
		fZmin = (float)fmin(fZmin, pProductInstance->m_fZmin);
		fZmax = (float)fmax(fZmax, pProductInstance->m_fZmax);
	} // for (size_t iInstance = ...
}

// ------------------------------------------------------------------------------------------------
void CProductDefinition::CalculateMinMaxTransform(CProductInstance* pProductInstance, float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax)
{
	if (m_iVerticesCount == 0)
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
			for (int64_t iIndex = m_vecTriangles[iTriangle].first; iIndex < m_vecTriangles[iTriangle].first + m_vecTriangles[iTriangle].second; iIndex++)
			{
				VECTOR3 vecPoint =
				{
					getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH)],
					getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 1],
					getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 2]
				};

				OGLTransform(&vecPoint, pProductInstance->getTransformationMatrix(), &vecPoint);

				fXmin = (float)fmin(fXmin, vecPoint.x);
				fXmax = (float)fmax(fXmax, vecPoint.x);
				fYmin = (float)fmin(fYmin, vecPoint.y);
				fYmax = (float)fmax(fYmax, vecPoint.y);
				fZmin = (float)fmin(fZmin, vecPoint.z);
				fZmax = (float)fmax(fZmax, vecPoint.z);
			} // for (size_t iIndex = ...
		} // for (size_t iTriangle = ...
	} // if (!m_vecTriangles.empty())	

	/*
	* Conceptual faces polygons
	*/
	if (!m_vecConceptualFacesPolygons.empty())
	{
		for (size_t iPolygon = 0; iPolygon < m_vecConceptualFacesPolygons.size(); iPolygon++)
		{
			for (int64_t iIndex = m_vecConceptualFacesPolygons[iPolygon].first; iIndex < m_vecConceptualFacesPolygons[iPolygon].first + m_vecConceptualFacesPolygons[iPolygon].second; iIndex++)
			{
				if ((getIndices()[iIndex] == -1) || (getIndices()[iIndex] == -2))
				{
					continue;
				}

				VECTOR3 vecPoint =
				{
					getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH)],
					getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 1],
					getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 2]
				};

				OGLTransform(&vecPoint, pProductInstance->getTransformationMatrix(), &vecPoint);

				fXmin = (float)fmin(fXmin, vecPoint.x);
				fXmax = (float)fmax(fXmax, vecPoint.x);
				fYmin = (float)fmin(fYmin, vecPoint.y);
				fYmax = (float)fmax(fYmax, vecPoint.y);
				fZmin = (float)fmin(fZmin, vecPoint.z);
				fZmax = (float)fmax(fZmax, vecPoint.z);
			} // for (size_t iIndex = ...
		} // for (size_t iPolygon = ...
	} // if (!m_vecConceptualFacesPolygons.empty())

	/*
	* Lines
	*/
	if (!m_vecLines.empty())
	{
		for (size_t iPolygon = 0; iPolygon < m_vecLines.size(); iPolygon++)
		{
			for (int64_t iIndex = m_vecLines[iPolygon].first; iIndex < m_vecLines[iPolygon].first + m_vecLines[iPolygon].second; iIndex++)
			{
				if (getIndices()[iIndex] == -1)
				{
					continue;
				}

				VECTOR3 vecPoint =
				{
					getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH)],
					getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 1],
					getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 2]
				};

				OGLTransform(&vecPoint, pProductInstance->getTransformationMatrix(), &vecPoint);

				fXmin = (float)fmin(fXmin, vecPoint.x);
				fXmax = (float)fmax(fXmax, vecPoint.x);
				fYmin = (float)fmin(fYmin, vecPoint.y);
				fYmax = (float)fmax(fYmax, vecPoint.y);
				fZmin = (float)fmin(fZmin, vecPoint.z);
				fZmax = (float)fmax(fZmax, vecPoint.z);
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
			for (int64_t iIndex = m_vecPoints[iPolygon].first; iIndex < m_vecPoints[iPolygon].first + m_vecPoints[iPolygon].second; iIndex++)
			{
				VECTOR3 vecPoint =
				{
					getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH)],
					getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 1],
					getVertices()[(getIndices()[iIndex] * VERTEX_LENGTH) + 2]
				};

				OGLTransform(&vecPoint, pProductInstance->getTransformationMatrix(), &vecPoint);

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

// ------------------------------------------------------------------------------------------------
void CProductDefinition::ScaleAndCenter(float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion)
{
	if (m_iVerticesCount == 0)
	{
		return;
	}	

	/**
	* Vertices
	*/
	for (int_t iVertex = 0; iVertex < m_iVerticesCount; iVertex++)
	{
		// [0.0 -> X/Y/Zmin + X/Y/Zmax]
		m_pVertexBuffer[(iVertex * VERTEX_LENGTH)] -= fXmin;
		m_pVertexBuffer[(iVertex * VERTEX_LENGTH) + 1] -= fYmin;
		m_pVertexBuffer[(iVertex * VERTEX_LENGTH) + 2] -= fZmin;

		// center
		m_pVertexBuffer[(iVertex * VERTEX_LENGTH)] -= ((fXmax - fXmin) / 2.0f);
		m_pVertexBuffer[(iVertex * VERTEX_LENGTH) + 1] -= ((fYmax - fYmin) / 2.0f);
		m_pVertexBuffer[(iVertex * VERTEX_LENGTH) + 2] -= ((fZmax - fZmin) / 2.0f);

		// [-1.0 -> 1.0]
		m_pVertexBuffer[(iVertex * VERTEX_LENGTH)] /= (fResoltuion / 2.0f);
		m_pVertexBuffer[(iVertex * VERTEX_LENGTH) + 1] /= (fResoltuion / 2.0f);
		m_pVertexBuffer[(iVertex * VERTEX_LENGTH) + 2] /= (fResoltuion / 2.0f);
	}	

	/**
	* Instances
	*/
	for (size_t iInstance = 0; iInstance < m_vecProductInstances.size(); iInstance++)
	{
		m_vecProductInstances[iInstance]->ScaleAndCenter(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax, fResoltuion);
	}
}

// ------------------------------------------------------------------------------------------------
float* CProductDefinition::BuildVBOVertexBuffer()
{
	ASSERT(m_iVerticesCount > 0);

	/*
	* Faces Vertex Buffer
	*/
	float* pVBOVertices = new float[m_iVerticesCount * GEOMETRY_VBO_VERTEX_LENGTH];
	memset(pVBOVertices, 0, m_iVerticesCount * GEOMETRY_VBO_VERTEX_LENGTH * sizeof(float));

	for (int64_t iVertex = 0; iVertex < m_iVerticesCount; iVertex++)
	{
		pVBOVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 0] = m_pVertexBuffer[(iVertex * VERTEX_LENGTH) + 0];
		pVBOVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] = m_pVertexBuffer[(iVertex * VERTEX_LENGTH) + 1];
		pVBOVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] = m_pVertexBuffer[(iVertex * VERTEX_LENGTH) + 2];

		pVBOVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 3] = m_pVertexBuffer[(iVertex * VERTEX_LENGTH) + 3];
		pVBOVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 4] = m_pVertexBuffer[(iVertex * VERTEX_LENGTH) + 4];
		pVBOVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 5] = m_pVertexBuffer[(iVertex * VERTEX_LENGTH) + 5];
	} // for (size_t iIndex = ...		

	return pVBOVertices;
}

// ------------------------------------------------------------------------------------------------
int_t CProductDefinition::getExpressID() const
{
	return m_iExpressID;
}

// ------------------------------------------------------------------------------------------------
int_t CProductDefinition::getInstance() const
{
	return m_iInstance;
}

// ------------------------------------------------------------------------------------------------
const vector<CProductInstance*>& CProductDefinition::getProductInstances() const
{
	return m_vecProductInstances;
}

// ------------------------------------------------------------------------------------------------
int CProductDefinition::getNextProductInstance()
{
	if (++m_iNextProductInstance >= m_vecProductInstances.size())
	{
		m_iNextProductInstance = 0;
	}

	return m_iNextProductInstance;
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CProductDefinition::getId() const
{
	return m_strId.c_str();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CProductDefinition::getName() const
{
	return m_strName.c_str();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CProductDefinition::getDescription() const
{
	return m_strDescription.c_str();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CProductDefinition::getProductId() const
{
	return m_strProductId.c_str();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CProductDefinition::getProductName() const
{
	return m_strProductName.c_str();
}

// ------------------------------------------------------------------------------------------------
int_t CProductDefinition::getRelatingProductRefs() const
{
	return m_iRelatingProductRefs;
}

// ------------------------------------------------------------------------------------------------
int_t CProductDefinition::getRelatedProductRefs() const
{
	return m_iRelatedProductRefs;
}

// ------------------------------------------------------------------------------------------------
int32_t* CProductDefinition::getIndices() const
{
	return m_pIndexBuffer;
}

int64_t CProductDefinition::getIndicesCount() const
{
	return m_iIndicesCount;
}

// ------------------------------------------------------------------------------------------------
float* CProductDefinition::getVertices() const
{
	return m_pVertexBuffer;
}

// ------------------------------------------------------------------------------------------------
int64_t CProductDefinition::getVerticesCount() const
{
	return m_iVerticesCount;
}

// ------------------------------------------------------------------------------------------------
int64_t CProductDefinition::getConceptualFacesCount() const
{
	return m_iConceptualFacesCount;
}

// ------------------------------------------------------------------------------------------------
const vector<pair<int64_t, int64_t> >& CProductDefinition::getTriangles() const
{
	return m_vecTriangles;
}

// ------------------------------------------------------------------------------------------------
const vector<pair<int64_t, int64_t> >& CProductDefinition::getLines() const
{
	return m_vecLines;
}

// ------------------------------------------------------------------------------------------------
const vector<pair<int64_t, int64_t> >& CProductDefinition::getPoints() const
{
	return m_vecPoints;
}

// ------------------------------------------------------------------------------------------------
const vector<pair<int64_t, int64_t> >& CProductDefinition::getConceptualFacesPolygons() const
{
	return m_vecConceptualFacesPolygons;
}

// ------------------------------------------------------------------------------------------------
vector<CSTEPGeometryWithMaterial*>& CProductDefinition::conceptualFacesMaterials()
{
	return m_vecMaterials;
}

// ------------------------------------------------------------------------------------------------
vector<CLinesCohort*>& CProductDefinition::linesCohorts()
{
	return m_vecLinesCohorts;
}

// ------------------------------------------------------------------------------------------------
vector<CPointsCohort*>& CProductDefinition::pointsCohorts()
{
	return m_vecPointsCohorts;
}

// ------------------------------------------------------------------------------------------------
vector<CWireframesCohort*>& CProductDefinition::conceptualFacesCohorts()
{
	return m_vecConceptualFacesCohorts;
}

// ------------------------------------------------------------------------------------------------
GLuint& CProductDefinition::VBO()
{
	return m_iVBO;
}

// ------------------------------------------------------------------------------------------------
GLsizei& CProductDefinition::VBOOffset()
{
	return m_iVBOOffset;
}

// ------------------------------------------------------------------------------------------------
void CProductDefinition::Clean()
{
	delete[] m_pVertexBuffer;
	m_pVertexBuffer = NULL;
	m_iVerticesCount = 0;

	delete[] m_pIndexBuffer;
	m_pIndexBuffer = NULL;
	m_iIndicesCount = 0;

	m_iConceptualFacesCount = 0;

	m_vecTriangles.clear();
	m_vecLines.clear();
	m_vecPoints.clear();
	m_vecConceptualFacesPolygons.clear();

	for (size_t iMaterial = 0; iMaterial < m_vecMaterials.size(); iMaterial++)
	{
		delete m_vecMaterials[iMaterial];
	}
	m_vecMaterials.clear();

	for (size_t iLinesCohort = 0; iLinesCohort < m_vecLinesCohorts.size(); iLinesCohort++)
	{
		delete m_vecLinesCohorts[iLinesCohort];
	}
	m_vecLinesCohorts.clear();

	for (size_t iPointsCohort = 0; iPointsCohort < m_vecPointsCohorts.size(); iPointsCohort++)
	{
		delete m_vecPointsCohorts[iPointsCohort];
	}
	m_vecPointsCohorts.clear();

	for (size_t iWireframesCohort = 0; iWireframesCohort < m_vecConceptualFacesCohorts.size(); iWireframesCohort++)
	{
		delete m_vecConceptualFacesCohorts[iWireframesCohort];
	}
	m_vecConceptualFacesCohorts.clear();
}
