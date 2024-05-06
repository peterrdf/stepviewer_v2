#include "stdafx.h"
#include "IFCInstance.h"
#include "IFCModel.h"

#include <math.h>

// ************************************************************************************************
CIFCInstance::CIFCInstance(int64_t iID, SdaiInstance iInstance)
	: _geometry(iID, iInstance, true)
	, CInstanceBase()
	, m_iInstance(iInstance)
	, m_iExpressID(0)
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

	Calculate();
}

// ------------------------------------------------------------------------------------------------
CIFCInstance::~CIFCInstance()
{
	Clean();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ SdaiInstance CIFCInstance::GetInstance() const
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

void CIFCInstance::Calculate()
{
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
		GetConceptualFaceEx(m_iInstance, iConceptualFace,
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
		_cohort* pCohort = ConcFacePolygonsCohorts().empty() ?
			nullptr : ConcFacePolygonsCohorts()[ConcFacePolygonsCohorts().size() - 1];

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
		auto pCohort = LinesCohorts().empty() ?
			nullptr : LinesCohorts()[LinesCohorts().size() - 1];

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
			} // for (int64_t iIndex = ...
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
int64_t CIFCInstance::ExpressID() const
{
	return m_iExpressID;
}

// ------------------------------------------------------------------------------------------------
float* CIFCInstance::getVertices()
{
	if (m_pVertexBuffer != nullptr)
	{
		return m_pVertexBuffer->data();
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
int64_t CIFCInstance::getVerticesCount()
{
	if (m_pVertexBuffer != nullptr)
	{
		return m_pVertexBuffer->size();
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
int64_t CIFCInstance::getVertexLength() const
{
	return _VERTEX_LENGTH;
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
int64_t& CIFCInstance::conceptualFacesCount()
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
vector<_cohortWithMaterial*>& CIFCInstance::ConcFacesCohorts()
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
vector<_cohortWithMaterial*>& CIFCInstance::PointsCohorts()
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
				fXmin = (float)fmin(fXmin, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH)]);
				fXmax = (float)fmax(fXmax, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH)]);
				fYmin = (float)fmin(fYmin, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 2]);
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

				fXmin = (float)fmin(fXmin, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH)]);
				fXmax = (float)fmax(fXmax, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH)]);
				fYmin = (float)fmin(fYmin, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 2]);
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

				fXmin = (float)fmin(fXmin, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH)]);
				fXmax = (float)fmax(fXmax, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH)]);
				fYmin = (float)fmin(fYmin, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 2]);
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
				fXmin = (float)fmin(fXmin, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH)]);
				fXmax = (float)fmax(fXmax, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH)]);
				fYmin = (float)fmin(fYmin, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 1]);
				fYmax = (float)fmax(fYmax, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 1]);
				fZmin = (float)fmin(fZmin, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 2]);
				fZmax = (float)fmax(fZmax, getVertices()[(GetIndices()[iIndex] * _VERTEX_LENGTH) + 2]);
			} // for (size_t iIndex = ...
		} // for (size_t iPolygon = ...
	} // if (!m_vecPoints.empty())
}

// ------------------------------------------------------------------------------------------------
void CIFCInstance::Scale(float fScaleFactor)
{
	if (!HasGeometry())
	{
		return;
	}

	// Vertices [-1.0 -> 1.0]
	for (int_t iVertex = 0; iVertex < m_pVertexBuffer->size(); iVertex++)
	{
		getVertices()[(iVertex * _VERTEX_LENGTH)] = getVertices()[(iVertex * _VERTEX_LENGTH)] / fScaleFactor;
		getVertices()[(iVertex * _VERTEX_LENGTH) + 1] = getVertices()[(iVertex * _VERTEX_LENGTH) + 1] / fScaleFactor;
		getVertices()[(iVertex * _VERTEX_LENGTH) + 2] = getVertices()[(iVertex * _VERTEX_LENGTH) + 2] / fScaleFactor;
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