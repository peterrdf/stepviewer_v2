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
	
	CIFCObject(CIFCModel * pIFCModel, int_t iInstance, const wchar_t * szGUID, int_t iEntity, const wchar_t * szEntity);
	~CIFCObject();
	
	int_t getInstance() const;
	const wchar_t* getGUID() const;
	int_t getEntity() const;
	const wchar_t* getEntityName() const;
	int64_t expressID() const;
	int64_t parentExpressID() const;
	int_t & ID();

	bool hasGeometry() const;

	CIFCModel* GetModel() const;
	
	float *& vertices();
	int_t & verticesCount();

	int64_t & conceptualFacesCount();

	vector<CIFCGeometryWithMaterial *> & conceptualFacesMaterials();

	vector<CLinesCohort *> & linesCohorts();
	vector<CWireframesCohort *> & wireframesCohorts();
	const CIFCMaterial * CIFCObject::getUserDefinedMaterial() const;
	void CIFCObject::setUserDefinedMaterial(CIFCMaterial * pIFCMaterial);

	const pair<float, float> & getXMinMax() const;
	const pair<float, float> & getYMinMax() const;
	const pair<float, float> & getZMinMax() const;

	bool& referenced();
	bool & visible__();
	bool & selectable__();

	void ShowFaces(BOOL bShow);
	BOOL AreFacesShown();
	void ShowLines(BOOL bShow);
	BOOL AreLinesShown();
	void ShowWireframes(BOOL bShow);
	BOOL AreWireframesShown();

	bool& selected();	
	
	GLuint & VBO();
	GLsizei & VBOOffset();
	
	void CalculateMinMaxValues(float & fXmin, float & fXmax, float & fYmin, float & fYmax, float & fZmin, float & fZmax);
	void CalculateMinMaxValues(float * pVertices, int_t iVerticesCount);
	void Scale(float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion);
	static void Scale(float * pVertices, int_t iVerticesCount, float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion);	
};

#endif // IFCOBJECT_H
