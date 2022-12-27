#ifndef IFCOBJECT_H
#define IFCOBJECT_H

#include <glew.h>

#include "STEPInstance.h"

#include "Generic.h"
#include "Cohorts.h"
#include "IFCMaterial.h"

#include "engine.h"
#include "ifcengine.h"

#include <cstddef>
#include <string>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CIFCObject : public CSTEPInstance
{
	// --------------------------------------------------------------------------------------------
	friend class CIFCModel;

private: // Members
	
	CIFCModel* m_pIFCModel;
	int_t m_iInstance;
	wstring m_strGUID;
	int_t m_iEntity;
	wstring m_strEntity;
	int64_t m_iExpressID;
	int64_t m_iParentExpressID;
	int_t m_iID;
	
	CIFCColor* m_pRGBID;

	float* m_pVertices;
	int_t m_iVerticesCount;
	
	int64_t m_iConceptualFacesCount;

	vector<CIFCGeometryWithMaterial *> m_vecConceptualFacesMaterials;

	
	vector<CLinesCohort *> m_vecLinesCohorts;
	vector<CWireframesCohort *> m_vecWireframesCohorts;

	CIFCMaterial * m_pUserDefinedMaterial;

	pair<float, float> m_prXMinMax;
	pair<float, float> m_prYMinMax;
	pair<float, float> m_prZMinMax;
	
	VECTOR3	m_Origin;
	VECTOR3	m_XVec;
	VECTOR3	m_YVec;
	VECTOR3	m_ZVec;
	
	float m_origin[3];
	float m_xDim[3];
	float m_yDim[3];
	float m_zDim[3];
	
	bool m_bReferenced;
	bool m_bVisible__;
	bool m_bSelectable__;

	BOOL m_bShowFaces;
	BOOL m_bShowWireframes;
	BOOL m_bShowLines;
	bool m_bSelected;

	// VBO (OpenGL)
	GLuint m_iVBO;
	GLsizei m_iVBOOffset;

public: // Methods	

	// --------------------------------------------------------------------------------------------
	// ctor
	CIFCObject(CIFCModel * pIFCModel, int_t iInstance, const wchar_t * szGUID, int_t iEntity, const wchar_t * szEntity);

	// --------------------------------------------------------------------------------------------
	// dtor
	~CIFCObject();

	// --------------------------------------------------------------------------------------------
	// Getter
	int_t getInstance() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const wchar_t* getGUID() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	int_t getEntity() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const wchar_t* getEntityName() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	int64_t expressID() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	int64_t parentExpressID() const;

	// --------------------------------------------------------------------------------------------
	// Accessor
	int_t & ID();

	// --------------------------------------------------------------------------------------------
	// Accessor
	CIFCColor * rgbID();

	// --------------------------------------------------------------------------------------------
	// Getter
	bool hasGeometry() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	CIFCModel * GetModel() const;

	// --------------------------------------------------------------------------------------------
	// Accessor
	float *& vertices();

	// --------------------------------------------------------------------------------------------
	// Accessor
	int_t & verticesCount();

	// --------------------------------------------------------------------------------------------
	// Accessor
	int64_t & conceptualFacesCount();

	// --------------------------------------------------------------------------------------------
	// Accessor
	vector<CIFCGeometryWithMaterial *> & conceptualFacesMaterials();

	// --------------------------------------------------------------------------------------------
	// Accessor
	vector<CLinesCohort *> & linesCohorts();

	// --------------------------------------------------------------------------------------------
	// Accessor
	vector<CWireframesCohort *> & wireframesCohorts();

	// --------------------------------------------------------------------------------------------
	// Getter
	const CIFCMaterial * CIFCObject::getUserDefinedMaterial() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void CIFCObject::setUserDefinedMaterial(CIFCMaterial * pIFCMaterial);

	// --------------------------------------------------------------------------------------------
	// Getter
	const pair<float, float> & getXMinMax() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const pair<float, float> & getYMinMax() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const pair<float, float> & getZMinMax() const;

	// --------------------------------------------------------------------------------------------
	// Accessor
	bool& referenced();

	// --------------------------------------------------------------------------------------------
	// Accessor
	bool & visible__();

	// --------------------------------------------------------------------------------------------
	// Accessor
	bool & selectable__();

	// --------------------------------------------------------------------------------------------
	// Faces
	void ShowFaces(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Faces 
	BOOL AreFacesShown();

	// --------------------------------------------------------------------------------------------
	// Lines
	void ShowLines(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Lines 
	BOOL AreLinesShown();

	// --------------------------------------------------------------------------------------------
	// Wireframes
	void ShowWireframes(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Wireframes 
	BOOL AreWireframesShown();

	// --------------------------------------------------------------------------------------------
	// Accessor
	bool & selected();

	// --------------------------------------------------------------------------------------------
	void SetMinimumBBox(float * origin, float * xDim, float * yDim, float * zDim);

	// --------------------------------------------------------------------------------------------
	// Getter
	VECTOR3 * getOrigin();

	// --------------------------------------------------------------------------------------------
	// Setter
	void setOrigin(VECTOR3 * vec);

	// --------------------------------------------------------------------------------------------
	// Getter
	VECTOR3 * getXVec();

	// --------------------------------------------------------------------------------------------
	// Setter
	void setXVec(VECTOR3 * vec);

	// --------------------------------------------------------------------------------------------
	// Getter
	VECTOR3 * getYVec();

	// --------------------------------------------------------------------------------------------
	// Setter
	void setYVec(VECTOR3 * vec);

	// --------------------------------------------------------------------------------------------
	// Getter
	VECTOR3 * getZVec();

	// --------------------------------------------------------------------------------------------
	// Setter
	void setZVec(VECTOR3 * vec);

	// --------------------------------------------------------------------------------------------
	// Getter
	float * getBBoxOrigin();

	// --------------------------------------------------------------------------------------------
	// Setter
	void setBBoxOrigin(VECTOR3 * vec);

	// --------------------------------------------------------------------------------------------
	// Getter
	float * getBBoxX();

	// --------------------------------------------------------------------------------------------
	// Setter
	void setBBoxX(VECTOR3 * vec);

	// --------------------------------------------------------------------------------------------
	// Getter
	float * getBBoxY();

	// --------------------------------------------------------------------------------------------
	// Setter
	void setBBoxY(VECTOR3 * vec);

	// --------------------------------------------------------------------------------------------
	// Getter
	float * getBBoxZ();

	// --------------------------------------------------------------------------------------------
	// Setter
	void setBBoxZ(VECTOR3 * vec);

	// ----------------------------------------------------------------------------
	// Bounding box
	void UpdateBBox(float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion);

	// --------------------------------------------------------------------------------------------
	// Accessor
	GLuint & VBO();

	// --------------------------------------------------------------------------------------------
	// Accessor
	GLsizei & VBOOffset();

	// --------------------------------------------------------------------------------------------
	// X/Y/Z min/max
	void CalculateMinMaxValues(float & fXmin, float & fXmax, float & fYmin, float & fYmax, float & fZmin, float & fZmax);	

	// --------------------------------------------------------------------------------------------
	// X/Y/Z min/max
	void CalculateMinMaxValues(float * pVertices, int_t iVerticesCount);

	// ----------------------------------------------------------------------------
	// X/Y/Z => [-1, 1]
	void Scale(float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion);

	// ----------------------------------------------------------------------------
	// X/Y/Z => [-1, 1]
	static void Scale(float * pVertices, int_t iVerticesCount, float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion);	
};

#endif // IFCOBJECT_H
