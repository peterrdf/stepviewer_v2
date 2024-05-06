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
	ExpressID m_iExpressID;
	
	// UI
	bool m_bReferenced;
	bool m_bEnable;

	// VBO (OpenGL)
	GLuint m_iVBO;
	GLsizei m_iVBOOffset;

public: // Methods
	
	CIFCInstance(int64_t iID, SdaiInstance iInstance);
	virtual ~CIFCInstance();

	// CInstanceBase
	virtual SdaiInstance GetInstance() const { return (SdaiInstance)m_iInstance; }
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
};

#endif // IFCINSTANCE_H
