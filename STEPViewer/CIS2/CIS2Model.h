#pragma once

#include "Model.h"
#include "CIS2DesignPart.h"
#include "CIS2Representation.h"

#include <string>
#include <map>
#include <set>
using namespace std;

// ************************************************************************************************
class CCIS2Model : public CModel
{

private: // Members

	// Load
	bool m_bLoadInstancesOnDemand;

	vector<CCIS2Instance*> m_vecInstances;
	map<SdaiInstance, CCIS2Instance*> m_mapInstances;
	map<int64_t, CCIS2Instance*> m_mapID2Instance;
	map<ExpressID, CCIS2Instance*> m_mapExpressID2Instance;

	CEntityProvider* m_pEntityProvider;

	static int_t s_iInstanceID;

	bool m_bUpdteVertexBuffers; // when the first instance with geometry is loaded

public: // Members

	static uint32_t DEFAULT_COLOR;

public: // Methods

	CCIS2Model(bool bLoadInstancesOnDemand = false);
	virtual ~CCIS2Model();

	// CModel
	virtual CEntityProvider* GetEntityProvider() const override { return m_pEntityProvider; }
	virtual CInstanceBase* GetInstanceByExpressID(ExpressID iExpressID) const override;
	virtual void ZoomToInstance(CInstanceBase* pInstance) override;
	virtual void ZoomOut() override;
	virtual CInstanceBase* LoadInstance(SdaiInstance iSdaiInstance) override;

	void Load(const wchar_t* szCIS2File, SdaiModel iModel);
	void PreLoadInstance(SdaiInstance iSdaiInstance);
	void Clean();

	void Scale(); // [-1, 1]

	const map<SdaiInstance, CCIS2Instance*>& GetInstances() const { return m_mapInstances; }
	CCIS2Instance* GetInstanceByID(ExpressID iID);

private: // Methods

	void LodDesignParts();
	void LoadRepresentations();
	CCIS2Instance* RetrieveGeometry(SdaiInstance iInstance, enumCIS2InstanceType enCIS2InstanceType, int_t iCircleSegments);
};

