#include "stdafx.h"
#include "ProductDefinition.h"
#include "Generic.h"

// ------------------------------------------------------------------------------------------------
CProductDefinition::CProductDefinition()
	: m_iExpressID(0)
	, m_iInstance(0)	
	, m_strId(L"")
	, m_strName(L"")
	, m_strDescription(L"")
	, m_strProductId(L"")
	, m_strProductName(L"")
	, m_iRelatingProductRefs(0)
	, m_iRelatedProductRefs(0)
	, m_vecProductInstances()
	, m_iNextProductInstance(-1)
	, m_pVertexBuffer(nullptr)
	, m_pIndexBuffer(nullptr)
	, m_iConceptualFacesCount(0)
	, m_bCalculated(false)
	, m_vecTriangles()
	, m_vecConcFacePolygons()
	, m_vecLines()
	, m_vecPoints()	
	, m_vecConcFacesCohorts()
	, m_vecConcFacePolygonsCohorts()
	, m_vecLinesCohorts()
	, m_vecPointsCohorts()
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
	mask += flagbit12;       // FACE POLYGON
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
	setting += 0;			 // FACE POLYGON OFF
	setting += flagbit13;    // CONCEPTUAL FACE POLYGON ON
	setting += flagbit24;	 //	AMBIENT
	setting += flagbit25;	 //	DIFFUSE
	setting += flagbit26;	 //	EMISSIVE
	setting += flagbit27;	 //	SPECULAR

	//	http://rdf.bg/gkdoc/CP64/SetFormat.html
	SetFormat(iModel, setting, mask);

	ASSERT(m_pVertexBuffer == nullptr);
	m_pVertexBuffer = new _vertices_f(_VERTEX_LENGTH);

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
	MATERIALS mapMaterial2ConcFacePoints; // MATERIAL : FACE INDEX, START INDEX, INIDCES COUNT, etc.

	//	http://rdf.bg/gkdoc/CP64/GetConceptualFaceCnt.html
	m_iConceptualFacesCount = GetConceptualFaceCnt(m_iInstance);
	for (int64_t iConceptualFace = 0; iConceptualFace < m_iConceptualFacesCount; iConceptualFace++)
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
		GetConceptualFace(m_iInstance, iConceptualFace,
			&iStartIndexTriangles, &iIndicesCountTriangles,
			&iStartIndexLines, &iIndicesCountLines,
			&iStartIndexPoints, &iIndicesCountPoints,
			0, 0,
			&iStartIndexConceptualFacePolygons, &iIndicesCountConceptualFacePolygons);

		if (iIndicesCountTriangles > 0)
		{
			/*
			* Material
			*/
			int32_t iIndexValue = *(m_pIndexBuffer->data() + iStartIndexTriangles);
			iIndexValue *= _VERTEX_LENGTH;

			float fColor = *(m_pVertexBuffer->data() + iIndexValue + 6);
			unsigned int iAmbientColor = *(reinterpret_cast<unsigned int*>(&fColor));
			float fTransparency = (float)(iAmbientColor & (255)) / (float)255;

			fColor = *(m_pVertexBuffer->data() + iIndexValue + 7);
			unsigned int iDiffuseColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(m_pVertexBuffer->data() + iIndexValue + 8);
			unsigned int iEmissiveColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(m_pVertexBuffer->data() + iIndexValue + 9);
			unsigned int iSpecularColor = *(reinterpret_cast<unsigned int*>(&fColor));

			/*
			* Material
			*/
			_material material(
				iAmbientColor,
				iDiffuseColor,
				iEmissiveColor,
				iSpecularColor,
				fTransparency,
				nullptr);

			auto itMaterial2ConceptualFaces = mapMaterial2ConcFaces.find(material);
			if (itMaterial2ConceptualFaces == mapMaterial2ConcFaces.end())
			{
				mapMaterial2ConcFaces[material] = vector<_face>{ _face(iConceptualFace, iStartIndexTriangles, iIndicesCountTriangles) };
			}
			else
			{
				itMaterial2ConceptualFaces->second.push_back(_face(iConceptualFace, iStartIndexTriangles, iIndicesCountTriangles));
			}
		}

		if (iIndicesCountTriangles > 0)
		{
			m_vecTriangles.push_back(_primitives(iStartIndexTriangles, iIndicesCountTriangles));
		}

		if (iIndicesCountConceptualFacePolygons > 0)
		{
			m_vecConcFacePolygons.push_back(_primitives(iStartIndexConceptualFacePolygons, iIndicesCountConceptualFacePolygons));
		}

		if (iIndicesCountLines > 0)
		{
			m_vecLines.push_back(_primitives(iStartIndexLines, iIndicesCountLines));
		}

		if (iIndicesCountPoints > 0)
		{
			int32_t iIndexValue = *(m_pIndexBuffer->data() + iStartIndexTriangles);
			iIndexValue *= _VERTEX_LENGTH;

			float fColor = *(m_pVertexBuffer->data() + iIndexValue + 6);
			unsigned int iAmbientColor = *(reinterpret_cast<unsigned int*>(&fColor));
			float fTransparency = (float)(iAmbientColor & (255)) / (float)255;

			fColor = *(m_pVertexBuffer->data() + iIndexValue + 7);
			unsigned int iDiffuseColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(m_pVertexBuffer->data() + iIndexValue + 8);
			unsigned int iEmissiveColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(m_pVertexBuffer->data() + iIndexValue + 9);
			unsigned int iSpecularColor = *(reinterpret_cast<unsigned int*>(&fColor));

			/*
			* Material
			*/
			_material material(
				iAmbientColor,
				iDiffuseColor,
				iEmissiveColor,
				iSpecularColor,
				fTransparency,
				nullptr);

			auto itMaterial2ConcFacePoints = mapMaterial2ConcFacePoints.find(material);
			if (itMaterial2ConcFacePoints == mapMaterial2ConcFacePoints.end())
			{
				mapMaterial2ConcFacePoints[material] = vector<_face>{ _face(iConceptualFace, iStartIndexPoints, iIndicesCountPoints) };
			}
			else
			{
				itMaterial2ConcFacePoints->second.push_back(_face(iConceptualFace, iStartIndexPoints, iIndicesCountPoints));
			}

			m_vecPoints.push_back(_primitives(iStartIndexPoints, iIndicesCountPoints));
		} // if (iIndicesCountPoints > 0)
	} // for (int64_t iConceptualFace = ...	

	/*
	* Group the faces
	*/
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

					ConcFacesCohorts().push_back(pNewCohort);

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

					ConcFacesCohorts().push_back(pNewCohort);

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

				ConcFacesCohorts().push_back(pCohort);
			}

			/*
			* Check the limit
			*/
			if (pCohort->indices().size() + iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohortWithMaterial(itMaterial2ConcFaces->first);

				ConcFacesCohorts().push_back(pCohort);
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

	/*
	* Group the polygons
	*/
	if (!m_vecConcFacePolygons.empty())
	{
		/*
		* Use the last cohort (if any)
		*/
		_cohort* pCohort = ConcFacePolygonsCohorts().empty() ? nullptr : ConcFacePolygonsCohorts()[ConcFacePolygonsCohorts().size() - 1];

		/*
		* Create the cohort
		*/
		if (pCohort == nullptr)
		{
			pCohort = new _cohort();
			ConcFacePolygonsCohorts().push_back(pCohort);
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
					ConcFacePolygonsCohorts().push_back(pCohort);

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
					ConcFacePolygonsCohorts().push_back(pCohort);

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
				ConcFacePolygonsCohorts().push_back(pCohort);
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

	/*
	* Group the lines
	*/
	if (!m_vecLines.empty())
	{
		/*
		* Use the last cohort (if any)
		*/
		auto pCohort = LinesCohorts().empty() ? nullptr : LinesCohorts()[LinesCohorts().size() - 1];

		/*
		* Create the cohort
		*/
		if (pCohort == nullptr)
		{
			pCohort = new _cohort();
			LinesCohorts().push_back(pCohort);
		}

		for (size_t iFace = 0; iFace < m_vecLines.size(); iFace++)
		{
			int64_t iStartIndex = m_vecLines[iFace].startIndex();
			int64_t iIndicesCount = m_vecLines[iFace].indicesCount();

			/*
			* Check the limit
			*/
			if (pCohort->indices().size() + iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohort();
				LinesCohorts().push_back(pCohort);
			}

			for (int64_t iIndex = iStartIndex; 
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				if (m_pIndexBuffer->data()[iIndex] < 0)
				{
					continue;
				}

				pCohort->indices().push_back(m_pIndexBuffer->data()[iIndex]);
			} // for (int_t iIndex = ...
		} // for (size_t iFace = ...
	} // if (!m_vecLines.empty())		

	/*
	* Group the points
	*/
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

					PointsCohorts().push_back(pNewCohort);

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

					PointsCohorts().push_back(pNewCohort);

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

				PointsCohorts().push_back(pCohort);
			}

			/*
			* Check the limit
			*/
			if (pCohort->indices().size() + iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohortWithMaterial(itMaterial2ConcFacePoints->first);

				PointsCohorts().push_back(pCohort);
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

// ------------------------------------------------------------------------------------------------
void CProductDefinition::CalculateMinMaxTransform(
	float& fXmin, float& fXmax, 
	float& fYmin, float& fYmax, 
	float& fZmin, float& fZmax)
{
	if (getVerticesCount() == 0)
	{
		return;
	}

	for (auto pInstance : m_vecProductInstances)
	{
		CalculateMinMaxTransform(
			pInstance,
			fXmin, fXmax,
			fYmin, fYmax,
			fZmin, fZmax);
	}
}

// ------------------------------------------------------------------------------------------------
void CProductDefinition::CalculateMinMaxTransform(
	float fXTranslation, float fYTranslation, float fZTranslation,
	float& fXmin, float& fXmax, 
	float& fYmin, float& fYmax, 
	float& fZmin, float& fZmax)
{
	if (getVerticesCount() == 0)
	{
		return;
	}

	for (auto pInstance : m_vecProductInstances)
	{
		CalculateMinMaxTransform(
			pInstance,
			fXTranslation, fYTranslation, fZTranslation,
			fXmin, fXmax,
			fYmin, fYmax,
			fZmin, fZmax);
	}
}

// ------------------------------------------------------------------------------------------------
void CProductDefinition::CalculateMinMaxTransform(
	CProductInstance* pInstance,
	float fXTranslation, float fYTranslation, float fZTranslation,
	float& fXmin, float& fXmax,
	float& fYmin, float& fYmax,
	float& fZmin, float& fZmax)
{
	double d_41 = pInstance->GetTransformationMatrix()->_41;
	double d_42 = pInstance->GetTransformationMatrix()->_42;
	double d_43 = pInstance->GetTransformationMatrix()->_43;

	pInstance->GetTransformationMatrix()->_41 += fXTranslation;
	pInstance->GetTransformationMatrix()->_42 += fYTranslation;
	pInstance->GetTransformationMatrix()->_43 += fZTranslation;

	CalculateMinMaxTransform(
		pInstance,
		fXmin, fXmax,
		fYmin, fYmax,
		fZmin, fZmax);

	pInstance->GetTransformationMatrix()->_41 = d_41;
	pInstance->GetTransformationMatrix()->_42 = d_42;
	pInstance->GetTransformationMatrix()->_43 = d_43;
}

// ------------------------------------------------------------------------------------------------
void CProductDefinition::CalculateMinMaxTransform(
	CProductInstance* pInstance,
	float& fXmin, float& fXmax, 
	float& fYmin, float& fYmax, 
	float& fZmin, float& fZmax)
{
	if (getVerticesCount() == 0)
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
				VECTOR3 vecPoint =
				{
					getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH)],
					getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 1],
					getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 2]
				};

				if (pInstance != nullptr)
				{
					OGLTransform(&vecPoint, pInstance->GetTransformationMatrix(), &vecPoint);
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

				VECTOR3 vecPoint =
				{
					getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH)],
					getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 1],
					getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 2]
				};

				if (pInstance != nullptr)
				{
					OGLTransform(&vecPoint, pInstance->GetTransformationMatrix(), &vecPoint);
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

				VECTOR3 vecPoint =
				{
					getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH)],
					getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 1],
					getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 2]
				};

				if (pInstance != nullptr)
				{
					OGLTransform(&vecPoint, pInstance->GetTransformationMatrix(), &vecPoint);
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
				VECTOR3 vecPoint =
				{
					getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH)],
					getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 1],
					getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 2]
				};

				if (pInstance != nullptr)
				{
					OGLTransform(&vecPoint, pInstance->GetTransformationMatrix(), &vecPoint);
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

// ------------------------------------------------------------------------------------------------
void CProductDefinition::ScaleAndCenter(
	float fXmin, float fXmax, 
	float fYmin, float fYmax, 
	float fZmin, float fZmax, 
	float fResoltuion)
{
	if (getVerticesCount() == 0)
	{
		return;
	}	

	/**
	* Vertices
	*/
	for (int_t iVertex = 0; iVertex < getVerticesCount(); iVertex++)
	{
		// [0.0 -> X/Y/Zmin + X/Y/Zmax]
		m_pVertexBuffer->data()[(iVertex * _VERTEX_LENGTH)] -= fXmin;
		m_pVertexBuffer->data()[(iVertex * _VERTEX_LENGTH) + 1] -= fYmin;
		m_pVertexBuffer->data()[(iVertex * _VERTEX_LENGTH) + 2] -= fZmin;

		// center
		m_pVertexBuffer->data()[(iVertex * _VERTEX_LENGTH)] -= ((fXmax - fXmin) / 2.0f);
		m_pVertexBuffer->data()[(iVertex * _VERTEX_LENGTH) + 1] -= ((fYmax - fYmin) / 2.0f);
		m_pVertexBuffer->data()[(iVertex * _VERTEX_LENGTH) + 2] -= ((fZmax - fZmin) / 2.0f);

		// [-1.0 -> 1.0]
		m_pVertexBuffer->data()[(iVertex * _VERTEX_LENGTH)] /= (fResoltuion / 2.0f);
		m_pVertexBuffer->data()[(iVertex * _VERTEX_LENGTH) + 1] /= (fResoltuion / 2.0f);
		m_pVertexBuffer->data()[(iVertex * _VERTEX_LENGTH) + 2] /= (fResoltuion / 2.0f);
	}	

	/**
	* Instances
	*/
	for (size_t iInstance = 0; iInstance < m_vecProductInstances.size(); iInstance++)
	{
		m_vecProductInstances[iInstance]->Scale(fResoltuion);
	}
}

// ------------------------------------------------------------------------------------------------
void CProductDefinition::Scale(float fResoltuion)
{
	if (getVerticesCount() == 0)
	{
		return;
	}

	/**
	* Vertices
	*/
	for (int_t iVertex = 0; iVertex < getVerticesCount(); iVertex++)
	{
		// [-1.0 -> 1.0]
		m_pVertexBuffer->data()[(iVertex * _VERTEX_LENGTH)] /= (fResoltuion / 2.0f);
		m_pVertexBuffer->data()[(iVertex * _VERTEX_LENGTH) + 1] /= (fResoltuion / 2.0f);
		m_pVertexBuffer->data()[(iVertex * _VERTEX_LENGTH) + 2] /= (fResoltuion / 2.0f);
	}

	/**
	* Instances
	*/
	for (size_t iInstance = 0; iInstance < m_vecProductInstances.size(); iInstance++)
	{
		m_vecProductInstances[iInstance]->Scale(fResoltuion);
	}
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CProductDefinition::GetId() const
{
	return m_strId.c_str();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CProductDefinition::GetName() const
{
	return m_strName.c_str();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CProductDefinition::GetDescription() const
{
	return m_strDescription.c_str();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CProductDefinition::GetProductId() const
{
	return m_strProductId.c_str();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CProductDefinition::GetProductName() const
{
	return m_strProductName.c_str();
}

// ------------------------------------------------------------------------------------------------
int_t CProductDefinition::GetRelatingProductRefs() const
{
	return m_iRelatingProductRefs;
}

// ------------------------------------------------------------------------------------------------
int_t CProductDefinition::GetRelatedProductRefs() const
{
	return m_iRelatedProductRefs;
}

// ------------------------------------------------------------------------------------------------
const vector<CProductInstance*>& CProductDefinition::GetInstances() const
{
	return m_vecProductInstances;
}

// ------------------------------------------------------------------------------------------------
int CProductDefinition::GetNextProductInstance()
{
	if (++m_iNextProductInstance >= (int)m_vecProductInstances.size())
	{
		m_iNextProductInstance = 0;
	}

	return m_iNextProductInstance;
}

// ------------------------------------------------------------------------------------------------
bool CProductDefinition::HasGeometry() const
{
	return (m_pVertexBuffer != nullptr) && (m_pVertexBuffer->size() > 0);
}

// ------------------------------------------------------------------------------------------------
int32_t* CProductDefinition::GetIndices() const
{
	if (m_pIndexBuffer != nullptr)
	{
		return m_pIndexBuffer->data();
	}

	return nullptr;
}

int64_t CProductDefinition::GetIndicesCount() const
{
	if (m_pIndexBuffer != nullptr)
	{
		return m_pIndexBuffer->size();
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
float* CProductDefinition::getVertices() const
{
	if (m_pVertexBuffer != nullptr)
	{
		return m_pVertexBuffer->data();
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
int64_t CProductDefinition::getVerticesCount() const
{
	if (m_pVertexBuffer != nullptr)
	{
		return m_pVertexBuffer->size();
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
int64_t CProductDefinition::getVertexLength() const
{
	return _VERTEX_LENGTH;
}

// ------------------------------------------------------------------------------------------------
int64_t CProductDefinition::GetConceptualFacesCount() const
{
	return m_iConceptualFacesCount;
}

// ------------------------------------------------------------------------------------------------
const vector<_primitives >& CProductDefinition::GetTriangles() const
{
	return m_vecTriangles;
}

// ------------------------------------------------------------------------------------------------
const vector<_primitives >& CProductDefinition::GetLines() const
{
	return m_vecLines;
}

// ------------------------------------------------------------------------------------------------
const vector<_primitives >& CProductDefinition::GetPoints() const
{
	return m_vecPoints;
}

// ------------------------------------------------------------------------------------------------
const vector<_primitives >& CProductDefinition::GetConcFacesPolygons() const
{
	return m_vecConcFacePolygons;
}

// ------------------------------------------------------------------------------------------------
vector<_cohortWithMaterial*>& CProductDefinition::ConcFacesCohorts()
{
	return m_vecConcFacesCohorts;
}

vector<_cohort*>& CProductDefinition::ConcFacePolygonsCohorts()
{
	return m_vecConcFacePolygonsCohorts;
}

// ------------------------------------------------------------------------------------------------
vector<_cohort*>& CProductDefinition::LinesCohorts()
{
	return m_vecLinesCohorts;
}

// ------------------------------------------------------------------------------------------------
vector<_cohortWithMaterial*>& CProductDefinition::PointsCohorts()
{
	return m_vecPointsCohorts;
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
