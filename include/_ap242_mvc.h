#pragma once

#include "_ap_mvc.h"

// ************************************************************************************************
class _ap242_product_definition;
class _ap242_assembly;
class _ap242_draughting_model;
class _ap242_annotation_plane;
class _ap242_draughting_callout;

// ************************************************************************************************
class _ap242_model : public _ap_model
{

private: // Members

	map<ExpressID, _ap242_assembly*> m_mapExpressIDAssembly; // Express ID : Assembly
	vector<_ap242_draughting_model*> m_vecDraughtingModels;

public: // Methods

	_ap242_model();
	virtual ~_ap242_model();

protected: // Methods

	// _ap_model
	virtual void attachModelCore() override;
	virtual void preLoadInstance(OwlInstance owlInstance) override;
	virtual void clean() override;

private: // Methods

	void loadProductDefinitions();
	_ap242_product_definition* loadProductDefinition(SdaiInstance iProductDefinitionInstance);
	_ap242_product_definition* getProductDefinition(SdaiInstance iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct);
	void loadAssemblies();
	void loadGeometry();
	void walkAssemblyTreeRecursively(_ap242_product_definition* pProductDefinition, _ap242_assembly* pAssembly, _matrix4x3* pParentMatrix);

	void loadDraughtingModels();
	_ap242_annotation_plane* loadAnnotationPlane(SdaiInstance sdaiInstance);
	_ap242_draughting_callout* loadDraughtingCallout(SdaiInstance sdaiInstance);	

	void Save(const wchar_t* szPath);	

public: // Properties

	const map<ExpressID, _ap242_assembly*>& getAssemblies() const { return m_mapExpressIDAssembly; }
	const vector<_ap242_draughting_model*>& getDraughtingModels() const { return m_vecDraughtingModels; }
};