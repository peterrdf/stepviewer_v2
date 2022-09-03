#pragma once

#include "IFCPropertyRestriction.h"

#include <string>
#include <vector>

using namespace std;

// ------------------------------------------------------------------------------------------------
// Class
class CIFCClass
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// Instance
	int64_t m_iInstance;

	// --------------------------------------------------------------------------------------------
	// Name
	wstring m_strName;

	// --------------------------------------------------------------------------------------------
	// Parents
	vector<int64_t> m_vecParentClasses;

	// --------------------------------------------------------------------------------------------
	// Parents
	vector<int64_t> m_vecAncestorClasses;

	// --------------------------------------------------------------------------------------------
	// Parents
	vector<CIFCPropertyRestriction *> m_vecPropertyRestrictions;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CIFCClass(int64_t iInstance);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CIFCClass();

	// --------------------------------------------------------------------------------------------
	// Getter
	int64_t getInstance() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const wchar_t * getName() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const vector<int64_t> & getParentClasses();

	// --------------------------------------------------------------------------------------------
	// Getter
	const vector<int64_t> & getAncestorClasses();

	// --------------------------------------------------------------------------------------------
	// Adds an RDF Property
	void AddPropertyRestriction(CIFCPropertyRestriction * pRDFPropertyRestriction);

	// --------------------------------------------------------------------------------------------
	// Getter
	const vector<CIFCPropertyRestriction *> & getPropertyRestrictions();

public: // Methods

	// --------------------------------------------------------------------------------------------
	// Helper
	static void GetAncestors(int64_t iClassInstance, vector<int64_t> & vecAncestorClasses);
};

