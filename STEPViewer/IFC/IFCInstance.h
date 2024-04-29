#ifndef IFCINSTANCE_H
#define IFCINSTANCE_H

#include "_oglUtils.h"

#include <glew.h>

#include "InstanceBase.h"

#include "Generic.h"

#include "engine.h"
#include "stepengine.h"

#include <cstddef>
#include <string>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CIFCInstance : public CInstanceBase
{
	// --------------------------------------------------------------------------------------------
	friend class CIFCModel;

private: // Members
	
	CIFCModel* m_pModel;
	SdaiInstance m_iInstance;
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
	
	CIFCInstance(CIFCModel* pModel, SdaiInstance iInstance, const wchar_t* szGUID);
	~CIFCInstance();

	// CInstanceBase
	virtual SdaiInstance GetInstance() const;
	virtual bool HasGeometry() const;
	virtual bool IsEnabled() const;

	const wchar_t* GetGUID() const;
	int64_t ExpressID() const;
	int64_t ParentExpressID() const;
	int_t& ID();

	CIFCModel* GetModel() const;
	
	float* GetVertices();
	int_t GetVerticesCount();
	int64_t GetVertexLength() const;
	int32_t* GetIndices() const;
	int64_t GetIndicesCount() const;
	int64_t& conceptualFacesCount();

	const vector<_primitives>& GetTriangles() const;
	const vector<_primitives>& GetLines() const;
	const vector<_primitives>& GetPoints() const;
	const vector<_primitives>& GetConcFacesPolygons() const;

	// Materials
	vector<_facesCohort*>& ConcFacesCohorts();

	// Cohorts
	vector<_cohort*>& ConcFacePolygonsCohorts();
	vector<_cohort*>& LinesCohorts();
	vector<_facesCohort*>& PointsCohorts();

	// State
	bool& Referenced();
	bool GetEnable() const;
	void SetEnable(bool bEnable);
	
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
