#pragma once

#include "_ifc_instance.h"
#include "_ifc_model.h"

#include <string>
#include <vector>
using namespace std;

// ************************************************************************************************
#define NODE_DECOMPOSITION	L"decomposition"
#define NODE_CONTAINS		L"contains"

// ************************************************************************************************
class _ifc_node
{

private: // Members

	SdaiInstance m_sdaiInstance;
	vector<_ifc_node*> m_vecChildren;

public: // Methods

	_ifc_node(SdaiInstance sdaiInstance);
	virtual ~_ifc_node();

public: // Properties

	SdaiInstance getSdaiInstance() const { return m_sdaiInstance; }
	virtual wchar_t* getGlobalId() const;
	vector<_ifc_node*>& children() { return m_vecChildren; }
};

// ************************************************************************************************
class _ifc_decomposition_node : public _ifc_node
{
public: // Methods

	_ifc_decomposition_node(SdaiInstance sdaiInstance);
	virtual ~_ifc_decomposition_node();

public: // Properties

	virtual wchar_t* getGlobalId() const override { return NODE_DECOMPOSITION; }
};

// ************************************************************************************************
class _ifc_contains_node : public _ifc_node
{

public: // Methods

	_ifc_contains_node(SdaiInstance sdaiInstance);
	virtual ~_ifc_contains_node();

public: // Properties

	virtual wchar_t* getGlobalId() const override { return NODE_CONTAINS; }
};

// ************************************************************************************************
class _ifc_model_structure
{

private: // Members

	_ifc_model* m_pModel;
	_ifc_node* m_pRootNode;

public: // Methods

	_ifc_model_structure(_ifc_model* pModel);
	virtual ~_ifc_model_structure();

protected: // Methods

	void build();
	void loadProjectNode(SdaiInstance sdaiProjectInstance);
	void loadIsDecomposedBy(_ifc_node* pParent, SdaiInstance sdaiInstance);
	void loadIsNestedBy(_ifc_node* pParent, SdaiInstance sdaiInstance);
	void loadContainsElements(_ifc_node* pParent, SdaiInstance sdaiInstance);
	void loadInstance(_ifc_node* pParent, SdaiInstance sdaiInstance);
	void clean();

public: // Properties

	_ifc_model* getModel() const { return m_pModel; }
	_ifc_node* getRootNode() const { return m_pRootNode; }
};

