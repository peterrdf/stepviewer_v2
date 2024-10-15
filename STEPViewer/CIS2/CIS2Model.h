#pragma once

#include "Model.h"

#include <string>
#include <map>
#include <set>
using namespace std;

// ************************************************************************************************
class CCIS2Model : public CModel
{

private: // Members

	CEntityProvider* m_pEntityProvider;

public: // Methods

	CCIS2Model();
	virtual ~CCIS2Model();

	// CModel
	virtual CEntityProvider* GetEntityProvider() const override { return m_pEntityProvider; }
	virtual CInstanceBase* GetInstanceByExpressID(ExpressID iExpressID) const override;
	virtual void ZoomToInstance(CInstanceBase* pInstance) override;
	virtual void ZoomOut() override;
	virtual CInstanceBase* LoadInstance(OwlInstance iInstance) override;

	void Load(const wchar_t* szCIS2File, SdaiModel iModel);
	void Clean();
};

