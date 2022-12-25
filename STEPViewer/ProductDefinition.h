#pragma once

#include "_oglUtils.h"

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
class CProductDefinition : public CSTEPInstance
{
	friend class CSTEPModel;

private: // Members

	int_t m_iExpressID;
	int_t m_iInstance;
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
	bool m_bCalculated;	
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
	
	bool m_bEnable; // UI

	// VBO (OpenGL)
	GLuint m_iVBO;
	GLsizei m_iVBOOffset;

public: // Methods

	CProductDefinition();
	virtual ~CProductDefinition();
	
	void Calculate();
	
	void CalculateMinMax(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax);
	void CalculateMinMaxTransform(float & fXmin, float & fXmax, float & fYmin, float & fYmax, float & fZmin, float & fZmax);
	void CalculateMinMaxTransform(CProductInstance* pProductInstance, float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax);
	void ScaleAndCenter(float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion);
	
	int_t getExpressID() const;
	int_t getInstance() const;	
	const wchar_t* getId() const;
	const wchar_t* getName() const;
	const wchar_t* getDescription() const;
	const wchar_t* getProductId() const;
	const wchar_t* getProductName() const;

	int_t getRelatingProductRefs() const;
	int_t getRelatedProductRefs() const;
	const vector<CProductInstance*>& getProductInstances() const;
	int getNextProductInstance();
	
	int32_t * getIndices() const;
	int64_t getIndicesCount() const;
	float * getVertices() const;
	int64_t getVerticesCount() const;
	int64_t getVertexLength() const;
	int64_t getConceptualFacesCount() const;

	const vector<_primitives> & getTriangles() const;
	const vector<_primitives> & getLines() const;
	const vector<_primitives> & getPoints() const;
	const vector<_primitives> & getConcFacesPolygons() const;

	// Materials
	vector<_facesCohort*>& concFacesCohorts();

	// Cohorts
	vector<_cohort*>& concFacePolygonsCohorts();
	vector<_cohort*>& linesCohorts();
	vector<_facesCohort*>& pointsCohorts();	

	GLuint& VBO();
	GLsizei& VBOOffset();

private: // Methods

	// --------------------------------------------------------------------------------------------
	void Clean();
};

