#pragma once

// ************************************************************************************************
//class CAP242ProductDefinition;

// ************************************************************************************************
class CAP242DraughtingModel
{

private: // Members

	SdaiInstance m_sdaiInstance;

public: // Methods
	
	CAP242DraughtingModel(SdaiInstance sdaiInstance);
	virtual ~CAP242DraughtingModel();

public: // Properties

	//CAP242ProductDefinition* GetProductDefinition() const;
};

