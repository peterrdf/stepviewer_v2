#pragma once

#include <string>
#include <vector>

using namespace std;

#include "engdef.h"

#include "ConceptualFace.h"

// ------------------------------------------------------------------------------------------------
class CIFCColor
{

private: // Members

	// --------------------------------------------------------------------------------------------
	float m_fR;

	// --------------------------------------------------------------------------------------------
	float m_fG;

	// --------------------------------------------------------------------------------------------
	float m_fB;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CIFCColor();

	// --------------------------------------------------------------------------------------------
	// dtor
	~CIFCColor();

	// --------------------------------------------------------------------------------------------
	// Set up
	void Init(unsigned int iColor)
	{
		m_fR = (float)(iColor & ((unsigned int)255 * 256 * 256 * 256)) / (256 * 256 * 256);
		m_fR /= 255.f;

		m_fG = (float)(iColor & (255 * 256 * 256)) / (256 * 256);
		m_fG /= 255.f;

		m_fB = (float)(iColor & (255 * 256)) / 256;
		m_fB /= 255.f;
	}

	// --------------------------------------------------------------------------------------------
	// Set up
	void Init(float fR, float fG, float fB)
	{
		m_fR = fR;
		m_fG = fG;
		m_fB = fB;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	float R() const
	{
		return m_fR;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	float G() const
	{
		return m_fG;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	float B() const
	{
		return m_fB;
	}

	// --------------------------------------------------------------------------------------------
	// operator
	bool operator == (const CIFCColor& c) const
	{
		return 
			((m_fR == c.m_fR) &&
			(m_fG == c.m_fG) &&
			(m_fB == c.m_fB));
	}

	// --------------------------------------------------------------------------------------------
	// operator
	bool operator < (const CIFCColor& c) const
	{
		if (m_fR < c.m_fR)
		{
			return true;
		}

		if (m_fR > c.m_fR)
		{
			return false;
		}		

		if (m_fG < c.m_fG)
		{
			return true;
		}

		if (m_fG > c.m_fG)
		{
			return false;
		}

		if (m_fB < c.m_fB)
		{
			return true;
		}

		if (m_fB > c.m_fB)
		{
			return false;
		}

		return false;
	}

	// --------------------------------------------------------------------------------------------
	// operator
	bool operator > (const CIFCColor& c) const
	{
		if (m_fR > c.m_fR)
		{
			return true;
		}

		if (m_fR < c.m_fR)
		{
			return false;
		}

		if (m_fG > c.m_fG)
		{
			return true;
		}

		if (m_fG < c.m_fG)
		{
			return false;
		}

		if (m_fB > c.m_fB)
		{
			return true;
		}

		if (m_fB < c.m_fB)
		{
			return false;
		}

		return false;
	}
};

// ------------------------------------------------------------------------------------------------
class CIFCMaterial
{
	// --------------------------------------------------------------------------------------------
	friend class CIFCModel;

private: // Members

	// --------------------------------------------------------------------------------------------
	// Color
	CIFCColor m_clrAmbient;

	// --------------------------------------------------------------------------------------------
	// Color
	CIFCColor m_clrDiffuse;

	// --------------------------------------------------------------------------------------------
	// Color
	CIFCColor m_clrEmissive;

	// --------------------------------------------------------------------------------------------
	// Color
	CIFCColor m_clrSpecular;

	// --------------------------------------------------------------------------------------------
	float m_fA;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CIFCMaterial();

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CIFCMaterial();

	// --------------------------------------------------------------------------------------------
	// Set up
	void Init(unsigned int iAmbientColor, unsigned int iDiffuseColor, unsigned int iEmissiveColor, unsigned int iSpecularColor, float fTransparency)
	{
		if ((iAmbientColor == 0) && (iDiffuseColor == 0) && (iEmissiveColor == 0) && (iSpecularColor == 0) && (fTransparency == 0.f))
		{
			/*
			* There is no material - use non-transparent black
			*/
			m_clrAmbient.Init(0);
			m_clrDiffuse.Init(0);
			m_clrEmissive.Init(0);
			m_clrSpecular.Init(0);

			m_fA = 1.f;
		}
		else
		{
			m_clrAmbient.Init(iAmbientColor);
			m_clrDiffuse.Init(iDiffuseColor == 0 ? iAmbientColor : iDiffuseColor);
			m_clrEmissive.Init(iEmissiveColor);
			m_clrSpecular.Init(iSpecularColor);

			m_fA = fTransparency;
		}
	}

	// --------------------------------------------------------------------------------------------
	// Set up
	void Init(BYTE ambientR, BYTE ambientG, BYTE ambientB, BYTE diffuseR, BYTE diffuseG, BYTE diffuseB, BYTE emissiveR, BYTE emissiveG, BYTE emissiveB, BYTE specularR, BYTE specularG, BYTE specularB, BYTE transparency)
	{
		m_clrAmbient.Init(ambientR / 255.f, ambientG / 255.f, ambientB / 255.f);
		m_clrDiffuse.Init(diffuseR / 255.f, diffuseG / 255.f, diffuseB / 255.f);
		m_clrEmissive.Init(emissiveR / 255.f, emissiveG / 255.f, emissiveB / 255.f);
		m_clrSpecular.Init(specularR / 255.f, specularG / 255.f, specularB / 255.f);

		m_fA = transparency / 255.f;
	}

	void Set(float ambientR, float ambientG, float ambientB, float diffuseR, float diffuseG, float diffuseB, float emissiveR, float emissiveG, float emissiveB, float specularR, float specularG, float specularB, float transparency)
	{
		m_clrAmbient.Init(ambientR, ambientG, ambientB);
		m_clrDiffuse.Init(diffuseR, diffuseG, diffuseB);
		m_clrEmissive.Init(emissiveR, emissiveG, emissiveB);
		m_clrSpecular.Init(specularR, specularG, specularB);

		m_fA = transparency;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const CIFCColor & getAmbientColor() const
	{
		return m_clrAmbient;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const CIFCColor & getDiffuseColor() const
	{
		return m_clrDiffuse;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const CIFCColor & getEmissiveColor() const
	{
		return m_clrEmissive;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const CIFCColor & getSpecularColor() const
	{
		return m_clrSpecular;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	float A() const
	{
		return m_fA;
	}
};

// ------------------------------------------------------------------------------------------------
class CIFCGeometryWithMaterial
{
	// --------------------------------------------------------------------------------------------
	friend class CIFCModel;

private: // Members

	CIFCMaterial * m_pMaterial;

	// --------------------------------------------------------------------------------------------
	// Conceptual faces
	vector<CConceptualFace> m_vecConceptualFaces;

	// --------------------------------------------------------------------------------------------
	// Geometry
	vector<GLuint> m_vecIndices;

	// --------------------------------------------------------------------------------------------
	// IBO
	GLuint m_iIBO;

	// --------------------------------------------------------------------------------------------
	// IBO - Offset
	GLsizei m_iIBOOffset;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CIFCGeometryWithMaterial();

	// --------------------------------------------------------------------------------------------
	// ctor
	CIFCGeometryWithMaterial(unsigned int iAmbientColor, unsigned int iDiffuseColor, unsigned int iEmissiveColor, unsigned int iSpecularColor, float fTransparency);

	// --------------------------------------------------------------------------------------------
	// ctor
	CIFCGeometryWithMaterial(const CIFCGeometryWithMaterial& material);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CIFCGeometryWithMaterial();

	// --------------------------------------------------------------------------------------------
	// Getter
	const CIFCMaterial * getMaterial() const
	{
		return m_pMaterial;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	void  setMaterial(CIFCMaterial * pMaterial)
	{
		m_pMaterial->Set(
				pMaterial->getAmbientColor().R(),
				pMaterial->getAmbientColor().G(),
				pMaterial->getAmbientColor().B(),
				pMaterial->getDiffuseColor().R(),
				pMaterial->getDiffuseColor().G(),
				pMaterial->getDiffuseColor().B(),
				pMaterial->getEmissiveColor().R(),
				pMaterial->getEmissiveColor().G(),
				pMaterial->getEmissiveColor().B(),
				pMaterial->getSpecularColor().R(),
				pMaterial->getSpecularColor().G(),
				pMaterial->getSpecularColor().B(),
				pMaterial->A()
			);
	}

	// --------------------------------------------------------------------------------------------
	// Accessor
	vector<CConceptualFace> & conceptualFaces()
	{
		return m_vecConceptualFaces;
	}

	// --------------------------------------------------------------------------------------------
	// Setter
	void addIndex(GLuint index)
	{
		m_vecIndices.push_back(index);
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	GLuint * getIndices()
	{
		return &(m_vecIndices[0]);
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	int_t getIndicesCount()
	{
		return m_vecIndices.size();
	}

	// --------------------------------------------------------------------------------------------
	// Accessor
	GLuint & IBO()
	{
		return m_iIBO;
	}

	// --------------------------------------------------------------------------------------------
	// Accessor
	GLsizei &  IBOOffset()
	{
		return m_iIBOOffset;
	}
};

// ------------------------------------------------------------------------------------------------
class CIFCMaterialComparator
{

public:

	// --------------------------------------------------------------------------------------------
	bool operator()(const CIFCMaterial &left, const CIFCMaterial &right) const
	{
		if (left.getAmbientColor() < right.getAmbientColor())
		{
			return true;
		}

		if (left.getAmbientColor() > right.getAmbientColor())
		{
			return false;
		}

		if (left.getDiffuseColor() < right.getDiffuseColor())
		{
			return true;
		}

		if (left.getDiffuseColor() > right.getDiffuseColor())
		{
			return false;
		}

		if (left.getEmissiveColor() < right.getEmissiveColor())
		{
			return true;
		}

		if (left.getEmissiveColor() > right.getEmissiveColor())
		{
			return false;
		}

		if (left.getSpecularColor() < right.getSpecularColor())
		{
			return true;
		}

		if (left.getSpecularColor() > right.getSpecularColor())
		{
			return false;
		}

		if (left.A() < right.A())
		{
			return true;
		}

		if (left.A() > right.A())
		{
			return false;
		}

		return false;
	}
};

// ------------------------------------------------------------------------------------------------
class CIFCGeometryWithMaterialComparator
{

public:

	// --------------------------------------------------------------------------------------------
	bool operator()(const CIFCGeometryWithMaterial &left, const CIFCGeometryWithMaterial &right) const
	{	
		if (left.getMaterial()->getAmbientColor() < right.getMaterial()->getAmbientColor())
		{
			return true;
		}

		if (left.getMaterial()->getAmbientColor() > right.getMaterial()->getAmbientColor())
		{
			return false;
		}

		if (left.getMaterial()->getDiffuseColor() < right.getMaterial()->getDiffuseColor())
		{
			return true;
		}

		if (left.getMaterial()->getDiffuseColor() > right.getMaterial()->getDiffuseColor())
		{
			return false;
		}

		if (left.getMaterial()->getEmissiveColor() < right.getMaterial()->getEmissiveColor())
		{
			return true;
		}

		if (left.getMaterial()->getEmissiveColor() > right.getMaterial()->getEmissiveColor())
		{
			return false;
		}

		if (left.getMaterial()->getSpecularColor() < right.getMaterial()->getSpecularColor())
		{
			return true;
		}

		if (left.getMaterial()->getSpecularColor() > right.getMaterial()->getSpecularColor())
		{
			return false;
		}

		if (left.getMaterial()->A() < right.getMaterial()->A())
		{
			return true;
		}

		if (left.getMaterial()->A() > right.getMaterial()->A())
		{
			return false;
		}

		return false;
	}
};