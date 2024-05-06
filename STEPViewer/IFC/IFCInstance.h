#ifndef IFCINSTANCE_H
#define IFCINSTANCE_H

#include "_oglUtils.h"
#include "InstanceBase.h"
#include "Generic.h"

#include "engine.h"
#include "stepengine.h"

#include <cstddef>
#include <string>
using namespace std;

// ************************************************************************************************
class CIFCInstance
	: public _geometry
	, public CInstanceBase
{
	friend class CIFCModel;

private: // Members

	// Metadata
	SdaiInstance m_iInstance;
	ExpressID m_iExpressID;
	
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
	vector<_cohortWithMaterial*> m_vecConcFacesCohorts;

	// Cohorts
	vector<_cohort*> m_vecConcFacePolygonsCohorts;
	vector<_cohort*> m_vecLinesCohorts;
	vector<_cohortWithMaterial*> m_vecPointsCohorts;
	
	// UI
	bool m_bReferenced;
	bool m_bEnable;

	// VBO (OpenGL)
	GLuint m_iVBO;
	GLsizei m_iVBOOffset;

public: // Methods
	
	CIFCInstance(int64_t iID, SdaiInstance iInstance);
	~CIFCInstance();

	// CInstanceBase
	virtual SdaiInstance GetInstance() const;
	virtual bool HasGeometry() const;
	virtual bool IsEnabled() const;

	void Calculate();

	int64_t ExpressID() const;
	
	float* getVertices();
	int64_t getVerticesCount();
	int64_t getVertexLength() const;
	int32_t* GetIndices() const;
	int64_t GetIndicesCount() const;
	int64_t& conceptualFacesCount();

	const vector<_primitives>& GetTriangles() const;
	const vector<_primitives>& GetLines() const;
	const vector<_primitives>& GetPoints() const;
	const vector<_primitives>& GetConcFacesPolygons() const;

	// Materials
	vector<_cohortWithMaterial*>& ConcFacesCohorts();

	// Cohorts
	vector<_cohort*>& ConcFacePolygonsCohorts();
	vector<_cohort*>& LinesCohorts();
	vector<_cohortWithMaterial*>& PointsCohorts();

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

	void Scale(float fScaleFactor);

private: // Methods

	void Clean();
};

#endif // IFCINSTANCE_H
