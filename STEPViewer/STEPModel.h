#pragma once

#include "STEPModelBase.h"
#include "Generic.h"

#include <map>

using namespace std;

#define EMPTY_INSTANCE L"---<EMPTY>---"

// ------------------------------------------------------------------------------------------------
class CAssembly;
class CProductDefinition;
class CProductInstance;

// ------------------------------------------------------------------------------------------------
// Model
class CSTEPModel : public CSTEPModelBase
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// Model
	int_t m_iModel;

	// --------------------------------------------------------------------------------------------
	// Express ID : Product Definition
	map<int_t, CProductDefinition*> m_mapProductDefinitions;

	// --------------------------------------------------------------------------------------------
	// ID : Product Instance
	map<int_t, CProductInstance*> m_mapProductInstances;

	// --------------------------------------------------------------------------------------------
	// Express ID : Assembly
	map<int_t, CAssembly*> m_mapAssemblies;

	// --------------------------------------------------------------------------------------------
	// ID (1-based index)
	int64_t m_iID;

	// --------------------------------------------------------------------------------------------
	// World's dimensions
	float m_fXmin;
	float m_fXmax;
	float m_fYmin;
	float m_fYmax;
	float m_fZmin;
	float m_fZmax;	

	// --------------------------------------------------------------------------------------------
	// World's bounding sphere diameter
	float m_fBoundingSphereDiameter;

	// --------------------------------------------------------------------------------------------
	// World's translations - center
	float m_fXTranslation;
	float m_fYTranslation;
	float m_fZTranslation;

	// --------------------------------------------------------------------------------------------
	// Support for textures
	//CTexture * m_pDefaultTexture;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CSTEPModel();

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CSTEPModel();

	// CSTEPModelBase
	virtual int64_t GetInstance() const;
	virtual void ZoomToInstance(CSTEPInstance* pSTEPInstance);
	virtual void ZoomOut() override;

	void GetWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const;

	// --------------------------------------------------------------------------------------------
	// Getter
	void GetWorldTranslations(float& fXTranslation, float& fYTranslation, float& fZTranslation) const;

	// --------------------------------------------------------------------------------------------
	// Getter
	float GetBoundingSphereDiameter() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<int_t, CProductDefinition*>& getProductDefinitions();

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<int_t, CProductInstance*>& getProductInstances();

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<int_t, CAssembly*>& getAssemblies();

	// --------------------------------------------------------------------------------------------
	// Getter
	CProductInstance* getProductInstanceByID(int_t iID) const;

	// --------------------------------------------------------------------------------------------
	// [-1, 1]
	void ScaleAndCenter();

	// --------------------------------------------------------------------------------------------
	// Stores model
	void Save(const wchar_t * szPath);

	// --------------------------------------------------------------------------------------------
	// Loads a model
	void Load(const wchar_t * szPath);

	// --------------------------------------------------------------------------------------------
	// Loads a model
	void Load(const wchar_t* szPath, int64_t iModel);

private: // Methods

	// ------------------------------------------------------------------------------------------------
	void LoadProductDefinitions();

	// --------------------------------------------------------------------------------------------
	CProductDefinition* LoadProductDefinition(int_t iProductDefinitionInstance);

	// ------------------------------------------------------------------------------------------------
	CProductDefinition* GetProductDefinition(int_t iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct);

	// ------------------------------------------------------------------------------------------------
	void LoadAssemblies();

	// ------------------------------------------------------------------------------------------------
	void LoadGeometry();

	// ------------------------------------------------------------------------------------------------
	void WalkAssemblyTreeRecursively(const char* szStepName, const char* szGroupName, CProductDefinition* pProductDefinition, MATRIX* pParentMatrix);

	// --------------------------------------------------------------------------------------------
	// Clean up
	void Clean();
};

