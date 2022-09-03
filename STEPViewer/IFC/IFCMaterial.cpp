#include "stdafx.h"
#include "IFCMaterial.h"

// ------------------------------------------------------------------------------------------------
CIFCColor::CIFCColor()
	: m_fR(0.f)
	, m_fG(0.f)
	, m_fB(0.f)
{
}

// ------------------------------------------------------------------------------------------------
CIFCColor::~CIFCColor()
{
}

// ------------------------------------------------------------------------------------------------
CIFCMaterial::CIFCMaterial()
	: m_clrAmbient()
	, m_clrDiffuse()
	, m_clrEmissive()
	, m_clrSpecular()
	, m_fA(1.f)
{
}

CIFCMaterial::~CIFCMaterial()
{
}

// ------------------------------------------------------------------------------------------------
CIFCGeometryWithMaterial::CIFCGeometryWithMaterial()
	: m_pMaterial(NULL)
	, m_vecConceptualFaces()
	, m_vecIndices()
	, m_iIBO(0)
	, m_iIBOOffset(0)
{
}

// ------------------------------------------------------------------------------------------------
CIFCGeometryWithMaterial::CIFCGeometryWithMaterial(unsigned int iAmbientColor, unsigned int iDiffuseColor, unsigned int iEmissiveColor, unsigned int iSpecularColor, float fTransparency)
	: CIFCGeometryWithMaterial()
{
	m_pMaterial = new CIFCMaterial();
	m_pMaterial->Init(iAmbientColor, iDiffuseColor, iEmissiveColor, iSpecularColor, fTransparency);
}

CIFCGeometryWithMaterial::CIFCGeometryWithMaterial(const CIFCGeometryWithMaterial& material)
	: CIFCGeometryWithMaterial()
{
	m_pMaterial = new CIFCMaterial();
	m_pMaterial->Set(
		material.getMaterial()->getAmbientColor().R(),
		material.getMaterial()->getAmbientColor().G(),
		material.getMaterial()->getAmbientColor().B(),
		material.getMaterial()->getDiffuseColor().R(),
		material.getMaterial()->getDiffuseColor().G(),
		material.getMaterial()->getDiffuseColor().B(),
		material.getMaterial()->getEmissiveColor().R(),
		material.getMaterial()->getEmissiveColor().G(),
		material.getMaterial()->getEmissiveColor().B(),
		material.getMaterial()->getSpecularColor().R(),
		material.getMaterial()->getSpecularColor().G(),
		material.getMaterial()->getSpecularColor().B(),
		material.getMaterial()->A());
}

// ------------------------------------------------------------------------------------------------
CIFCGeometryWithMaterial::~CIFCGeometryWithMaterial()
{
	delete m_pMaterial;
}
