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
	, m_pRGBID(new CIFCColor())
	, m_pVertices(NULL)
	, m_iVerticesCount(0)
	, m_iConceptualFacesCount(0)
	, m_vecConceptualFacesMaterials()
	, m_vecLinesCohorts()
	, m_vecWireframesCohorts()
	, m_pUserDefinedMaterial(NULL)
	, m_prXMinMax(pair<float, float>(-1.f, 1.f))
	, m_prYMinMax(pair<float, float>(-1.f, 1.f))
	, m_prZMinMax(pair<float, float>(-1.f, 1.f))
	, m_vecConceptualFacesArea()
	, m_dVolume(0.)
	, m_bReferenced(false)
	, m_bVisible__(true)
	, m_bSelectable__(true)
	, m_bShowFaces(true)
	, m_bShowWireframes(true)
	, m_bShowLines(true)
	, m_bSelected(false)
	, m_iVBO(0)
	, m_iVBOOffset(0)
{
	m_Origin = { 0, 0., 0. };
	m_XVec = { 0, 0., 0. };
	m_YVec = { 0, 0., 0. };
	m_ZVec = { 0, 0., 0. };

	std::fill(std::begin(m_origin), std::end(m_origin), 0.f);
	std::fill(std::begin(m_xDim), std::end(m_xDim), 0.f);
	std::fill(std::begin(m_yDim), std::end(m_yDim), 0.f);
	std::fill(std::begin(m_zDim), std::end(m_zDim), 0.f);

	m_iExpressID = internalGetP21Line(m_iInstance);

	{
		int_t	m_iInstanceRelAggregates = 0;
		sdaiGetAttrBN(m_iInstance, "Decomposes", sdaiINSTANCE, &m_iInstanceRelAggregates);
		if (m_iInstanceRelAggregates) {
			int_t	m_iInstanceParent = 0;
			sdaiGetAttrBN(m_iInstanceRelAggregates, "RelatingObject", sdaiINSTANCE, &m_iInstanceParent);
			if (m_iInstanceParent) {
				m_iParentExpressID = internalGetP21Line(m_iInstanceParent);
			}
		}
	}
	
	{
		int_t	m_iInstanceRelContainedInSpatialStructure = 0;
		sdaiGetAttrBN(m_iInstance, "ContainedInStructure", sdaiINSTANCE, &m_iInstanceRelContainedInSpatialStructure);
		if (m_iInstanceRelContainedInSpatialStructure) {
			int_t	m_iInstanceParent = 0;
			sdaiGetAttrBN(m_iInstanceRelContainedInSpatialStructure, "RelatingStructure", sdaiINSTANCE, &m_iInstanceParent);
			if (m_iInstanceParent) {
				m_iParentExpressID = internalGetP21Line(m_iInstanceParent);
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
CIFCObject::~CIFCObject()
{
	delete[] m_pVertices;
	m_pVertices = NULL;

	for (size_t iMaterial = 0; iMaterial < m_vecConceptualFacesMaterials.size(); iMaterial++)
	{
		delete m_vecConceptualFacesMaterials[iMaterial];
	}
	m_vecConceptualFacesMaterials.clear();

	for (size_t iLinesCohort = 0; iLinesCohort < m_vecLinesCohorts.size(); iLinesCohort++)
	{
		delete m_vecLinesCohorts[iLinesCohort];
	}
	m_vecLinesCohorts.clear();	

	for (size_t iWireframesCohort = 0; iWireframesCohort < m_vecWireframesCohorts.size(); iWireframesCohort++)
	{
		delete m_vecWireframesCohorts[iWireframesCohort];
	}
	m_vecWireframesCohorts.clear();	

	delete m_pRGBID;
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
CIFCColor * CIFCObject::rgbID()
{
	return m_pRGBID;
}

// ------------------------------------------------------------------------------------------------
bool CIFCObject::hasGeometry() const
{
	return (m_pVertices != NULL) && (m_iVerticesCount > 0);
}

CIFCModel * CIFCObject::GetModel() const
{
	return m_pIFCModel;
}

// ------------------------------------------------------------------------------------------------
float *& CIFCObject::vertices()
{
	return m_pVertices;
}

// ------------------------------------------------------------------------------------------------
int_t & CIFCObject::verticesCount()
{
	return m_iVerticesCount;
}

// ------------------------------------------------------------------------------------------------
int_t & CIFCObject::conceptualFacesCount()
{
	return m_iConceptualFacesCount;
}

// ------------------------------------------------------------------------------------------------
vector<CIFCGeometryWithMaterial *> & CIFCObject::conceptualFacesMaterials()
{
	return m_vecConceptualFacesMaterials;
}

// ------------------------------------------------------------------------------------------------
vector<CLinesCohort *> & CIFCObject::linesCohorts()
{
	return m_vecLinesCohorts;
}

// ------------------------------------------------------------------------------------------------
vector<CWireframesCohort *> & CIFCObject::wireframesCohorts()
{
	return m_vecWireframesCohorts;
}

// ------------------------------------------------------------------------------------------------
const CIFCMaterial * CIFCObject::getUserDefinedMaterial() const
{
	return m_pUserDefinedMaterial;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::setUserDefinedMaterial(CIFCMaterial * pIFCMaterial)
{
	m_pUserDefinedMaterial = pIFCMaterial;
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
vector<double>& CIFCObject::conceptualFacesArea()
{
	return m_vecConceptualFacesArea;
}

// ------------------------------------------------------------------------------------------------
double CIFCObject::getConceptualFaceArea(int_t iFace)
{
	if ((iFace < 0) || (iFace >= (int_t)m_vecConceptualFacesArea.size()))
	{
		ASSERT(FALSE);

		return 0.;
	}

	return m_vecConceptualFacesArea[iFace];
}

// ------------------------------------------------------------------------------------------------
double& CIFCObject::volume()
{
	return m_dVolume;
}

// ------------------------------------------------------------------------------------------------
bool&  CIFCObject::referenced()
{
	return m_bReferenced;
}

// ------------------------------------------------------------------------------------------------
bool & CIFCObject::visible__()
{
	return m_bVisible__;
}

// ------------------------------------------------------------------------------------------------
bool & CIFCObject::selectable__()
{
	return m_bSelectable__;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::ShowFaces(BOOL bShow)
{
	m_bShowFaces = bShow;
}

// ------------------------------------------------------------------------------------------------
BOOL CIFCObject::AreFacesShown()
{
	return m_bShowFaces;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::ShowLines(BOOL bShow)
{
	m_bShowLines = bShow;
}

// ------------------------------------------------------------------------------------------------
BOOL CIFCObject::AreLinesShown()
{
	return m_bShowLines;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::ShowWireframes(BOOL bShow)
{
	m_bShowWireframes = bShow;
}

// ------------------------------------------------------------------------------------------------
BOOL CIFCObject::AreWireframesShown()
{
	return m_bShowWireframes;
}

// ------------------------------------------------------------------------------------------------
bool & CIFCObject::selected()
{
	return m_bSelected;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::SetMinimumBBox(float * origin, float * xDim, float * yDim, float * zDim)
{
	m_origin[0] = origin[0];
	m_origin[1] = origin[1];
	m_origin[2] = origin[2];

	m_xDim[0] = xDim[0];
	m_xDim[1] = xDim[1];
	m_xDim[2] = xDim[2];

	m_yDim[0] = yDim[0];
	m_yDim[1] = yDim[1];
	m_yDim[2] = yDim[2];

	m_zDim[0] = zDim[0];
	m_zDim[1] = zDim[1];
	m_zDim[2] = zDim[2];
}

// ------------------------------------------------------------------------------------------------
VECTOR3 * CIFCObject::getOrigin()
{
	return &m_Origin;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::setOrigin(VECTOR3 * vec)
{
	m_Origin.x = vec->x;
	m_Origin.y = vec->y;
	m_Origin.z = vec->z;
}

// ------------------------------------------------------------------------------------------------
VECTOR3 * CIFCObject::getXVec()
{
	return &m_XVec;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::setXVec(VECTOR3 * vec)
{
	m_XVec.x = vec->x;
	m_XVec.y = vec->y;
	m_XVec.z = vec->z;
}

// ------------------------------------------------------------------------------------------------
VECTOR3 * CIFCObject::getYVec()
{
	return &m_YVec;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::setYVec(VECTOR3 * vec)
{
	m_YVec.x = vec->x;
	m_YVec.y = vec->y;
	m_YVec.z = vec->z;
}

// ------------------------------------------------------------------------------------------------
VECTOR3 * CIFCObject::getZVec()
{
	return &m_ZVec;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::setZVec(VECTOR3 * vec)
{
	m_ZVec.x = vec->x;
	m_ZVec.y = vec->y;
	m_ZVec.z = vec->z;
}

// ------------------------------------------------------------------------------------------------
float * CIFCObject::getBBoxOrigin()
{
	return m_origin;
}

// --------------------------------------------------------------------------------------------
void CIFCObject::setBBoxOrigin(VECTOR3 * vec)
{
	m_origin[0] = (float)vec->x;
	m_origin[1] = (float)vec->y;
	m_origin[2] = (float)vec->z;
}

// ------------------------------------------------------------------------------------------------
float * CIFCObject::getBBoxX()
{
	return m_xDim;
}

// --------------------------------------------------------------------------------------------
void CIFCObject::setBBoxX(VECTOR3 * vec)
{
	m_xDim[0] = (float)vec->x;
	m_xDim[1] = (float)vec->y;
	m_xDim[2] = (float)vec->z;
}

// ------------------------------------------------------------------------------------------------
float * CIFCObject::getBBoxY()
{
	return m_yDim;
}

// --------------------------------------------------------------------------------------------
void CIFCObject::setBBoxY(VECTOR3 * vec)
{
	m_yDim[0] = (float)vec->x;
	m_yDim[1] = (float)vec->y;
	m_yDim[2] = (float)vec->z;
}

// ------------------------------------------------------------------------------------------------
float * CIFCObject::getBBoxZ()
{
	return m_zDim;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::setBBoxZ(VECTOR3 * vec)
{
	m_zDim[0] = (float)vec->x;
	m_zDim[1] = (float)vec->y;
	m_zDim[2] = (float)vec->z;
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::UpdateBBox(float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion)
{
	std::fill(std::begin(m_origin), std::end(m_origin), 0.f);
	std::fill(std::begin(m_xDim), std::end(m_xDim), 0.f);
	std::fill(std::begin(m_yDim), std::end(m_yDim), 0.f);
	std::fill(std::begin(m_zDim), std::end(m_zDim), 0.f);

	VECTOR3 origin = m_Origin;

	origin.x -= m_pIFCModel->getXoffset();
	origin.y -= m_pIFCModel->getYoffset();
	origin.z -= m_pIFCModel->getZoffset();

	m_origin[0] = (float)origin.x;
	m_origin[1] = (float)origin.z;
	m_origin[2] = (float)-origin.y;

	m_xDim[0] = (float)m_XVec.x;
	m_xDim[1] = (float)m_XVec.z;
	m_xDim[2] = (float)-m_XVec.y;

	m_yDim[0] = (float)m_YVec.x;
	m_yDim[1] = (float)m_YVec.z;
	m_yDim[2] = (float)-m_YVec.y;

	m_zDim[0] = (float)m_ZVec.x;
	m_zDim[1] = (float)m_ZVec.z;
	m_zDim[2] = (float)-m_ZVec.y;

	m_origin[0] -= (fXmin + fXmax) / 2;
	m_origin[1] -= (fYmin + fYmax) / 2;
	m_origin[2] -= (fZmin + fZmax) / 2;

	m_origin[0] /= (fResoltuion / 2.0f);
	m_origin[1] /= (fResoltuion / 2.0f);
	m_origin[2] /= (fResoltuion / 2.0f);

	m_xDim[0] /= (fResoltuion / 2.0f);
	m_xDim[1] /= (fResoltuion / 2.0f);
	m_xDim[2] /= (fResoltuion / 2.0f);

	m_yDim[0] /= (fResoltuion / 2.0f);
	m_yDim[1] /= (fResoltuion / 2.0f);
	m_yDim[2] /= (fResoltuion / 2.0f);

	m_zDim[0] /= (fResoltuion / 2.0f);
	m_zDim[1] /= (fResoltuion / 2.0f);
	m_zDim[2] /= (fResoltuion / 2.0f);
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
void CIFCObject::CalculateMinMaxValues(float & fXmin, float & fXmax, float & fYmin, float & fYmax, float & fZmin, float & fZmax)
{
	for (int_t iVertex = 0; iVertex < m_iVerticesCount; iVertex++)
	{
		fXmin = fmin(fXmin, m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)]);
		fXmax = fmax(fXmax, m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)]);

		fYmin = fmin(fYmin, m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1]);
		fYmax = fmax(fYmax, m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1]);

		fZmin = fmin(fZmin, m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2]);
		fZmax = fmax(fZmax, m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2]);
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCObject::CalculateMinMaxValues(float * pVertices, int_t iVerticesCount)
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
	if (m_iVerticesCount == 0)
	{
		ATLASSERT(FALSE);

		return;
	}

	for (int_t iVertex = 0; iVertex < m_iVerticesCount; iVertex++)
	{
		// [0.0 -> X/Y/Zmin + X/Y/Zmax]
		m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] = m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] - fXmin;
		m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] = m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] - fYmin;
		m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] = m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] - fZmin;

		// center
		m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] = m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] - ((fXmax - fXmin) / 2.0f);
		m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] = m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] - ((fYmax - fYmin) / 2.0f);
		m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] = m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] - ((fZmax - fZmin) / 2.0f);

		// [-1.0 -> 1.0]
		m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] = m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH)] / (fResoltuion / 2.0f);
		m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] = m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] / (fResoltuion / 2.0f);
		m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] = m_pVertices[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] / (fResoltuion / 2.0f);
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
