#pragma once

#include "Generic.h"
#include "Cohorts.h"
#include "ProductInstance.h"
#include "ConceptualFace.h"
#include "STEPInstance.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CSTEPColor
{

private: // Members

	// ------------------------------------------------------------------------------------------------
	float m_fR;

	// ------------------------------------------------------------------------------------------------
	float m_fG;

	// ------------------------------------------------------------------------------------------------
	float m_fB;

public: // Methods

	// ------------------------------------------------------------------------------------------------
	// ctor
	CSTEPColor()
		: m_fR(0.f)
		, m_fG(0.f)
		, m_fB(0.f)
	{
	}

	// ------------------------------------------------------------------------------------------------
	// ctor
	CSTEPColor(float fR, float fG, float fB)
		: m_fR(fR)
		, m_fG(fG)
		, m_fB(fB)
	{
	}

	// ------------------------------------------------------------------------------------------------
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

	// ------------------------------------------------------------------------------------------------
	// Getter
	float R() const
	{
		return m_fR;
	}

	// ------------------------------------------------------------------------------------------------
	// Getter
	float G() const
	{
		return m_fG;
	}

	// ------------------------------------------------------------------------------------------------
	// Getter
	float B() const
	{
		return m_fB;
	}

	// --------------------------------------------------------------------------------------------
	// operator
	bool operator == (const CSTEPColor& c) const
	{
		return
			((m_fR == c.m_fR) &&
				(m_fG == c.m_fG) &&
				(m_fB == c.m_fB));
	}

	// --------------------------------------------------------------------------------------------
	// operator
	bool operator < (const CSTEPColor& c) const
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
	bool operator > (const CSTEPColor& c) const
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
class CSTEPMaterial
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// Color
	CSTEPColor m_clrAmbient;

	// --------------------------------------------------------------------------------------------
	// Color
	CSTEPColor m_clrDiffuse;

	// --------------------------------------------------------------------------------------------
	// Color
	CSTEPColor m_clrEmissive;

	// --------------------------------------------------------------------------------------------
	// Color
	CSTEPColor m_clrSpecular;

	// ------------------------------------------------------------------------------------------------
	float m_fA;

	// ------------------------------------------------------------------------------------------------
	// Texture
	wstring m_strTexture;

public: // Methods

	// ------------------------------------------------------------------------------------------------
	// ctor
	CSTEPMaterial()
		: m_clrAmbient()
		, m_clrDiffuse()
		, m_clrEmissive()
		, m_clrSpecular()
		, m_fA(1.f)
		, m_strTexture(L"")
	{
	}

	// ------------------------------------------------------------------------------------------------
	// ctor
	CSTEPMaterial(unsigned int iAmbientColor, unsigned int iDiffuseColor, unsigned int iEmissiveColor, unsigned int iSpecularColor, float fTransparency)
		: m_clrAmbient()
		, m_clrDiffuse()
		, m_clrEmissive()
		, m_clrSpecular()
		, m_fA(1.f)
		, m_strTexture(L"")
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
	// Initialize
	void init(
		unsigned int iAmbientColor,
		unsigned int iDiffuseColor,
		unsigned int iEmissiveColor,
		unsigned int iSpecularColor,
		float fTransparency,
		const wchar_t* szTexture)
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

		if ((szTexture != NULL) && (wcslen(szTexture) > 0))
		{
			m_strTexture = szTexture;
		}
	}

	// --------------------------------------------------------------------------------------------
	// Initialize
	void set(float ambientR, float ambientG, float ambientB,
		float diffuseR, float diffuseG, float diffuseB,
		float emissiveR, float emissiveG, float emissiveB,
		float specularR, float specularG, float specularB,
		float transparency, const wchar_t* szTexture)
	{
		m_clrAmbient.Init(ambientR, ambientG, ambientB);
		m_clrDiffuse.Init(diffuseR, diffuseG, diffuseB);
		m_clrEmissive.Init(emissiveR, emissiveG, emissiveB);
		m_clrSpecular.Init(specularR, specularG, specularB);

		m_fA = transparency;

		if ((szTexture != NULL) && (wcslen(szTexture) > 0))
		{
			m_strTexture = szTexture;
		}
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const CSTEPColor& getAmbientColor() const
	{
		return m_clrAmbient;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const CSTEPColor& getDiffuseColor() const
	{
		return m_clrDiffuse;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const CSTEPColor& getEmissiveColor() const
	{
		return m_clrEmissive;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const CSTEPColor& getSpecularColor() const
	{
		return m_clrSpecular;
	}

	// ------------------------------------------------------------------------------------------------
	// Getter
	float A() const
	{
		return m_fA;
	}

	// ------------------------------------------------------------------------------------------------
	// Getter
	bool hasTexture() const
	{
		return m_strTexture.size() > 0;
	}

	// ------------------------------------------------------------------------------------------------
	// Getter
	const wchar_t* getTexture() const
	{
		return m_strTexture.c_str();
	}

	// ------------------------------------------------------------------------------------------------
	// Getter
	void setTexture(const wstring& strTexture)
	{
		m_strTexture = strTexture;
	}
};

// ------------------------------------------------------------------------------------------------
class CSTEPGeometryWithMaterial
{
	// --------------------------------------------------------------------------------------------
	friend class CSTEPModel;

private: // Members

	CSTEPMaterial* m_pMaterial;

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
	CSTEPGeometryWithMaterial()
		: m_pMaterial(NULL)
		, m_vecConceptualFaces()
		, m_vecIndices()
		, m_iIBO(0)
		, m_iIBOOffset(0)
	{
	}

	// --------------------------------------------------------------------------------------------
	// ctor
	CSTEPGeometryWithMaterial(
		unsigned int iAmbientColor,
		unsigned int iDiffuseColor,
		unsigned int iEmissiveColor,
		unsigned int iSpecularColor,
		float fTransparency,
		const wchar_t* szTexture)
		: CSTEPGeometryWithMaterial()
	{
		m_pMaterial = new CSTEPMaterial();
		m_pMaterial->init(iAmbientColor, iDiffuseColor, iEmissiveColor, iSpecularColor, fTransparency, szTexture);
	}

	// --------------------------------------------------------------------------------------------
	// ctor
	CSTEPGeometryWithMaterial(const CSTEPGeometryWithMaterial& material)
		: CSTEPGeometryWithMaterial()
	{
		m_pMaterial = new CSTEPMaterial();
		m_pMaterial->set(
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
			material.getMaterial()->A(),
			material.getMaterial()->getTexture());
	}

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CSTEPGeometryWithMaterial()
	{
		delete m_pMaterial;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const CSTEPMaterial* getMaterial() const
	{
		return m_pMaterial;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	void setMaterial(CSTEPMaterial* pMaterial)
	{
		m_pMaterial->set(
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
			pMaterial->A(),
			pMaterial->getTexture()
		);
	}

	// --------------------------------------------------------------------------------------------
	// Accessor
	vector<CConceptualFace>& conceptualFaces()
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
	GLuint* getIndices()
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
	GLuint& IBO()
	{
		return m_iIBO;
	}

	// --------------------------------------------------------------------------------------------
	// Accessor
	GLsizei& IBOOffset()
	{
		return m_iIBOOffset;
	}
};

// ------------------------------------------------------------------------------------------------
class CSTEPGeometryWithMaterialComparator
{

public:

	// --------------------------------------------------------------------------------------------
	bool operator()(const CSTEPGeometryWithMaterial& left, const CSTEPGeometryWithMaterial& right) const
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

		wstring strLeftTexture = left.getMaterial()->getTexture() != NULL ? left.getMaterial()->getTexture() : L"";
		wstring strRighttTexture = right.getMaterial()->getTexture() != NULL ? right.getMaterial()->getTexture() : L"";

		if (strLeftTexture < strRighttTexture)
		{
			return true;
		}

		if (strLeftTexture > strRighttTexture)
		{
			return false;
		}

		return false;
	}
};

// ------------------------------------------------------------------------------------------------
typedef map<CSTEPGeometryWithMaterial, vector<CConceptualFace>, CSTEPGeometryWithMaterialComparator> MATERIALS;

// ------------------------------------------------------------------------------------------------
class CProductDefinition : public CSTEPInstance
{
	// --------------------------------------------------------------------------------------------
	friend class CSTEPModel;

private: // Members

	// --------------------------------------------------------------------------------------------
	int_t m_iExpressID;

	// --------------------------------------------------------------------------------------------
	int_t m_iInstance;

	// --------------------------------------------------------------------------------------------
	vector<CProductInstance*> m_vecProductInstances;

	// --------------------------------------------------------------------------------------------
	int m_iNextProductInstance;

	// --------------------------------------------------------------------------------------------
	wstring m_strId;

	// --------------------------------------------------------------------------------------------
	wstring m_strName;

	// --------------------------------------------------------------------------------------------
	wstring m_strDescription;

	// --------------------------------------------------------------------------------------------
	wstring m_strProductId;

	// --------------------------------------------------------------------------------------------
	wstring m_strProductName;

	// --------------------------------------------------------------------------------------------
	// if == 0 then it has geometry, otherwise it is a placeholder
	int_t m_iRelatingProductRefs;

	// --------------------------------------------------------------------------------------------
	// if == 0 then it is a root element
	int_t m_iRelatedProductRefs;

	// --------------------------------------------------------------------------------------------
	bool m_bCalculated;

	// --------------------------------------------------------------------------------------------
	// Scaled & Centered Vertices - [-1, 1]
	float * m_pVertexBuffer;

	// --------------------------------------------------------------------------------------------
	// Vertices
	int_t m_iVerticesCount;

	// --------------------------------------------------------------------------------------------
	// Indices
	int32_t* m_pIndexBuffer;

	// --------------------------------------------------------------------------------------------
	// Indices
	int_t m_iIndicesCount;

	// --------------------------------------------------------------------------------------------
	// Conceptual faces count
	int64_t m_iConceptualFacesCount;

	// --------------------------------------------------------------------------------------------
	// Triangles
	vector<pair<int64_t, int64_t>> m_vecTriangles;

	// --------------------------------------------------------------------------------------------
	// Lines
	vector<pair<int64_t, int64_t>> m_vecLines;

	// --------------------------------------------------------------------------------------------
	// Points
	vector<pair<int64_t, int64_t>> m_vecPoints;

	// --------------------------------------------------------------------------------------------
	// Conceptual faces polygons
	vector<pair<int64_t, int64_t>> m_vecConceptualFacesPolygons;

	// --------------------------------------------------------------------------------------------
	// Materials
	vector<CSTEPGeometryWithMaterial*> m_vecMaterials;

	// --------------------------------------------------------------------------------------------
	// Lines
	vector<CLinesCohort*> m_vecLinesCohorts;

	// --------------------------------------------------------------------------------------------
	// Points
	vector<CPointsCohort*> m_vecPointsCohorts;	

	// --------------------------------------------------------------------------------------------
	// Wireframes
	vector<CWireframesCohort*> m_vecConceptualFacesCohorts;

	// --------------------------------------------------------------------------------------------
	// Enable/Disable
	bool m_bEnable;

	// --------------------------------------------------------------------------------------------
	// VBO
	GLuint m_iVBO;

	// --------------------------------------------------------------------------------------------
	// VBO - Offset
	GLsizei m_iVBOOffset;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CProductDefinition();

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CProductDefinition();

	// --------------------------------------------------------------------------------------------
	void Calculate();

	// --------------------------------------------------------------------------------------------
	// Helper
	void CalculateMinMax(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax);

	// --------------------------------------------------------------------------------------------
	// Helper
	void CalculateMinMaxTransform(float & fXmin, float & fXmax, float & fYmin, float & fYmax, float & fZmin, float & fZmax);

	// --------------------------------------------------------------------------------------------
	// Helper
	void CalculateMinMaxTransform(CProductInstance* pProductInstance, float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax);

	// --------------------------------------------------------------------------------------------
	void ScaleAndCenter(float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion);

	// --------------------------------------------------------------------------------------------
	float* BuildVBOVertexBuffer();

	// --------------------------------------------------------------------------------------------
	int_t getExpressID() const;

	// --------------------------------------------------------------------------------------------
	int_t getInstance() const;

	// --------------------------------------------------------------------------------------------
	const vector<CProductInstance*>& getProductInstances() const;

	// --------------------------------------------------------------------------------------------
	int getNextProductInstance();

	// --------------------------------------------------------------------------------------------
	const wchar_t* getId() const;

	// --------------------------------------------------------------------------------------------
	const wchar_t* getName() const;

	// --------------------------------------------------------------------------------------------
	const wchar_t* getDescription() const;

	// --------------------------------------------------------------------------------------------
	const wchar_t* getProductId() const;

	// --------------------------------------------------------------------------------------------
	const wchar_t* getProductName() const;

	// --------------------------------------------------------------------------------------------
	int_t getRelatingProductRefs() const;

	// --------------------------------------------------------------------------------------------
	int_t getRelatedProductRefs() const;

		// --------------------------------------------------------------------------------------------
	// Getter
	int32_t * getIndices() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	int64_t getIndicesCount() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	float * getVertices() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	int64_t getVerticesCount() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	int64_t getConceptualFacesCount() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const vector<pair<int64_t, int64_t>> & getTriangles() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const vector<pair<int64_t, int64_t>> & getLines() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const vector<pair<int64_t, int64_t>> & getPoints() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const vector<pair<int64_t, int64_t>> & getFacesPolygons() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const vector<pair<int64_t, int64_t>> & getConceptualFacesPolygons() const;

	// --------------------------------------------------------------------------------------------
	// Accessor
	vector<CSTEPGeometryWithMaterial*>& conceptualFacesMaterials();

	// --------------------------------------------------------------------------------------------
	// Accessor
	vector<CLinesCohort*>& linesCohorts();

	// --------------------------------------------------------------------------------------------
	// Accessor
	vector<CPointsCohort*>& pointsCohorts();	

	// --------------------------------------------------------------------------------------------
	// Accessor
	vector<CWireframesCohort*>& conceptualFacesCohorts();	

	// --------------------------------------------------------------------------------------------
	// Accessor
	GLuint& VBO();

	// --------------------------------------------------------------------------------------------
	// Accessor
	GLsizei& VBOOffset();

private: // Methods

	// --------------------------------------------------------------------------------------------
	void Clean();
};

