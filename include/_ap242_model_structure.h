#pragma once

#include "_ap242_model.h"

#include <string>
#include <vector>
using namespace std;

// ************************************************************************************************
class _ap242_node
{

private: // Members

	SdaiInstance m_sdaiInstance;
	_ap242_node* m_pParent;
	vector<_ap242_node*> m_vecChildren;

public: // Methods

	_ap242_node(SdaiInstance sdaiInstance, _ap242_node* pParentNode);
	virtual ~_ap242_node();

public: // Properties

	SdaiInstance getSdaiInstance() const { return m_sdaiInstance; }
	_ap242_node* getParent() const { return m_pParent; }
	virtual wchar_t* getGlobalId() const;
	vector<_ap242_node*>& children() { return m_vecChildren; }
};

// ************************************************************************************************
class _ap242_model_structure
{

private: // Members

	_ap242_model* m_pModel;
	vector<_ap242_node*> m_vecRootsProducts;

public: // Methods

	_ap242_model_structure(_ap242_model* pModel);
	virtual ~_ap242_model_structure();

	void build();

#ifdef _DEBUG
	void print(int iLevel = 0, _ap242_node* pNode = nullptr);
#endif

protected: // Methods

	void clean();

public: // Properties

	_ap242_model* getModel() const { return m_pModel; }
	const vector<_ap242_node*>& getRootsProducts() { return m_vecRootsProducts; }
};

