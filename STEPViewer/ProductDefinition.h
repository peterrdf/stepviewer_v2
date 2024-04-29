#pragma once

#include "_oglUtils.h"

#include "Generic.h"
#include "ProductInstance.h"
#include "InstanceBase.h"

#include <string>
#include <vector>
#include <map>
using namespace std;


// ************************************************************************************************
class CProductDefinition
{
	friend class CSTEPModel;

private: // Members

	ExpressID m_iExpressID;
	SdaiInstance m_iInstance;
	wstring m_strId;
	wstring m_strName;
	wstring m_strDescription;
	wstring m_strProductId;
	wstring m_strProductName;

	// Instances		
	int_t m_iRelatingProductRefs; // if == 0 then it has geometry, otherwise it is a placeholder
	int_t m_iRelatedProductRefs; // if == 0 then it is a root element
	vector<CProductInstance*> m_vecProductInstances;
	int m_iNextProductInstance;
	
	// Geometry
	_vertices_f* m_pVertexBuffer; // Scaled & Centered Vertices - [-1, 1]
	_indices_i32* m_pIndexBuffer;
	int64_t m_iConceptualFacesCount;
	bool m_bCalculated;

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

	// VBO (OpenGL)
	GLuint m_iVBO;
	GLsizei m_iVBOOffset;

public: // Methods

	// ctor/dtor
	CProductDefinition();
	virtual ~CProductDefinition();
	
	void Calculate();	
	
	void CalculateMinMaxTransform(
		float & fXmin, float & fXmax, 
		float & fYmin, float & fYmax, 
		float & fZmin, float & fZmax);

	void CalculateMinMaxTransform(
		float fXTranslation, float fYTranslation, float fZTranslation,
		float& fXmin, float& fXmax, 
		float& fYmin, float& fYmax, 
		float& fZmin, float& fZmax);

	void CalculateMinMaxTransform(
		CProductInstance* pInstance, 
		float& fXmin, float& fXmax, 
		float& fYmin, float& fYmax, 
		float& fZmin, float& fZmax);

	void CalculateMinMaxTransform(
		CProductInstance* pInstance,
		float fXTranslation, float fYTranslation, float fZTranslation,
		float& fXmin, float& fXmax,
		float& fYmin, float& fYmax,
		float& fZmin, float& fZmax);

	void ScaleAndCenter(
		float fXmin, float fXmax, 
		float fYmin, float fYmax, 
		float fZmin, float fZmax, 
		float fResoltuion);

	void Scale(float fResoltuion);
	
	ExpressID GetExpressID() const { return m_iExpressID; }
	SdaiInstance GetInstance() const { return m_iInstance; }
	const wchar_t* GetId() const;
	const wchar_t* GetName() const;
	const wchar_t* GetDescription() const;
	const wchar_t* GetProductId() const;
	const wchar_t* GetProductName() const;

	int_t GetRelatingProductRefs() const;
	int_t GetRelatedProductRefs() const;
	const vector<CProductInstance*>& GetInstances() const;
	int GetNextProductInstance();

	bool HasGeometry() const;
	
	int32_t* GetIndices() const;
	int64_t GetIndicesCount() const;
	float*  getVertices() const;
	int64_t getVerticesCount() const;
	int64_t getVertexLength() const;
	int64_t GetConceptualFacesCount() const;

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

	// VBO (OpenGL)
	GLuint& VBO();
	GLsizei& VBOOffset();

private: // Methods
	
	void Clean();
};

