#ifndef IFCOBJECT_H
#define IFCOBJECT_H

#include "_oglUtils.h"

#include <glew.h>

#include "STEPInstance.h"

#include "Generic.h"

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
	
	// Geometry
	_vertices_f* m_pVertexBuffer; // Scaled & Centered Vertices - [-1, 1]
	_indices_i32* m_pIndexBuffer;
	int64_t m_iConceptualFacesCount;

	// Primitives
	vector<_primitives> m_vecTriangles;
	vector<_primitives> m_vecConcFacePolygons;
	vector<_primitives> m_vecLines;
	vector<_primitives> m_vecPoints;

	// Materials
	vector<_facesCohort*> m_vecConcFacesCohorts;

	// Cohorts
	vector<_cohort*> m_vecConcFacePolygonsCohorts;
	vector<_cohort*> m_vecLinesCohorts;
	vector<_facesCohort*> m_vecPointsCohorts;

	// Model
	pair<float, float> m_prXMinMax;
	pair<float, float> m_prYMinMax;
	pair<float, float> m_prZMinMax;	
	
	// UI
	bool m_bReferenced;
	bool m_bEnable;
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
	
	float* getVertices();
	int_t getVerticesCount();
	int64_t getVertexLength() const;
	int32_t* getIndices() const;
	int64_t getIndicesCount() const;
	int64_t& conceptualFacesCount();

	const vector<_primitives>& getTriangles() const;
	const vector<_primitives>& getLines() const;
	const vector<_primitives>& getPoints() const;
	const vector<_primitives>& getConcFacesPolygons() const;

	// Materials
	vector<_facesCohort*>& concFacesCohorts();

	// Cohorts
	vector<_cohort*>& concFacePolygonsCohorts();
	vector<_cohort*>& linesCohorts();
	vector<_facesCohort*>& pointsCohorts();

	// State
	bool& referenced();
	bool getEnable() const;
	void setEnable(bool bEnable);
	void setSelected(bool bSelected);
	bool getSelected() const;
	
	GLuint & VBO();
	GLsizei & VBOOffset();
	
	void CalculateMinMax(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax);
	void ScaleAndCenter(float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion);

private: // Methods

	void Clean();
};

#endif // IFCOBJECT_H
