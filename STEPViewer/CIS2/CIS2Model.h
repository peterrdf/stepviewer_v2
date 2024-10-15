#pragma once

#include "Model.h"
#include "CIS2Representation.h"

#include <string>
#include <map>
#include <set>
using namespace std;

// ************************************************************************************************
class CCIS2Model : public CModel
{

private: // Members

	vector<CCIS2Representation*> m_vecInstances;
	map<SdaiInstance, CCIS2Representation*> m_mapInstances;
	map<int64_t, CCIS2Representation*> m_mapID2Instance;
	map<ExpressID, CCIS2Representation*> m_mapExpressID2Instance;

	CEntityProvider* m_pEntityProvider;

	static int_t s_iInstanceID;

	bool m_bUpdteVertexBuffers; // when the first instance with geometry is loaded

public: // Members

	static uint32_t DEFAULT_COLOR;

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
	void PreLoadInstance(SdaiInstance iInstance);
	void Clean();

	void Scale(); // [-1, 1]

	const map<SdaiInstance, CCIS2Representation*>& GetInstances() const { return m_mapInstances; }
	CCIS2Representation* GetInstanceByID(int64_t iID);

private: // Methods

	CCIS2Representation* RetrieveGeometry(SdaiInstance iInstance, int_t iCircleSegments);
};

