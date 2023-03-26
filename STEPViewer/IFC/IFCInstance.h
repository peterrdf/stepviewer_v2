#ifndef IFCINSTANCE_H
#define IFCINSTANCE_H

#include "_oglUtils.h"

#include <glew.h>

#include "Instance.h"

#include "Generic.h"

#include "engine.h"
#include "stepengine.h"

#include <cstddef>
#include <string>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CIFCInstance : public CInstance
{
	// --------------------------------------------------------------------------------------------
	friend class CIFCModel;

private: // Members
	
	CIFCModel* m_pModel;
	int_t m_iInstance;
	wstring m_strGUID;
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
	
	// UI
	bool m_bReferenced;
	bool m_bEnable;

	// VBO (OpenGL)
	GLuint m_iVBO;
	GLsizei m_iVBOOffset;

public: // Methods
	
	CIFCInstance(CIFCModel* pModel, int_t iInstance, const wchar_t* szGUID);
	~CIFCInstance();

	// CInstance
	virtual int64_t GetInstance() const;
	virtual bool HasGeometry() const;
	virtual bool IsEnabled() const;

	const wchar_t* getGUID() const;
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
	
	GLuint& VBO();
	GLsizei& VBOOffset();
	
	void CalculateMinMax(
		float& fXmin, float& fXmax, 
		float& fYmin, float& fYmax, 
		float& fZmin, float& fZmax);

	void ScaleAndCenter(
		float fXmin, float fXmax, 
		float fYmin, float fYmax, 
		float fZmin, float fZmax, 
		float fResoltuion,
		bool bScale);

private: // Methods

	void Clean();
};

#endif // IFCINSTANCE_H
