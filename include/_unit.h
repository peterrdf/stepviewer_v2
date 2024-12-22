#ifndef IFCUNIT_H
#define IFCUNIT_H

#include "engine.h"

#include <string>
#include <map>

using namespace std;

#define	UNKNOWN	0
#define	ABSORBEDDOSEUNIT	101
#define	AREAUNIT	102
#define	DOSEEQUIVALENTUNIT	103
#define	ELECTRICCAPACITANCEUNIT	104
#define	ELECTRICCHARGEUNIT	105
#define	ELECTRICCONDUCTANCEUNIT	106
#define	ELECTRICCURRENTUNIT	107
#define	ELECTRICRESISTANCEUNIT	108
#define	ELECTRICVOLTAGEUNIT	109
#define	ENERGYUNIT	110
#define	FORCEUNIT	111
#define	FREQUENCYUNIT	112
#define	ILLUMINANCEUNIT	113
#define	INDUCTANCEUNIT	114
#define	LENGTHUNIT	115
#define	LUMINOUSFLUXUNIT	116
#define	LUMINOUSINTENSITYUNIT	117
#define	MAGNETICFLUXDENSITYUNIT	118
#define	MAGNETICFLUXUNIT	119
#define	MASSUNIT	120
#define	PLANEANGLEUNIT	121
#define	POWERUNIT	122
#define	PRESSUREUNIT	123
#define	RADIOACTIVITYUNIT	124
#define	SOLIDANGLEUNIT	125
#define	THERMODYNAMICTEMPERATUREUNIT	126
#define	TIMEUNIT	127
#define	VOLUMEUNIT	128
#define	USERDEFINED	129

// ************************************************************************************************
class CIFCUnit
{

private: // Members	

	int m_iType;
	wstring m_strType;
	wstring m_strPrefix;
	wstring m_strName;

public: // Methods
	
	CIFCUnit(const wchar_t* szType, const wchar_t* szPrefix, const wchar_t* szName);
    virtual ~CIFCUnit();

	wstring GetType() const;
    wstring GetPrefix() const;
    wstring GetName() const;
    wstring GetUnit() const; // [PREFIX][SPACE][NAME]

protected: // Methods
	
    void ConvertType(const wchar_t* szUnitType);
	void ConvertPrefix(const wchar_t* szPrefix);
	void ConvertName(const wchar_t * szName);
};

// ************************************************************************************************
class CIFCUnitProvider
{

private: // Members

	SdaiModel m_iModel;
	map<wstring, CIFCUnit*> m_mapUnits;

public: // Methods

	CIFCUnitProvider(SdaiModel iModel);
	virtual ~CIFCUnitProvider();

	const CIFCUnit* GetUnit(const wchar_t* szUnit) const;
	
	pair<wstring, wstring> GetQuantity(SdaiInstance iQuantityInstance, const char* szValueName, const wchar_t* szUnitName) const;
	pair<wstring, wstring> GetQuantityLength(SdaiInstance iQuantityInstance) const;
	pair<wstring, wstring> GetQuantityArea(SdaiInstance iQuantityInstance) const;
	pair<wstring, wstring> GetQuantityVolume(SdaiInstance iQuantityInstance) const;
	pair<wstring, wstring> GetQuantityCount(SdaiInstance iQuantityInstance) const;
	pair<wstring, wstring> GetQuantityWeight(SdaiInstance iQuantityInstance) const;
	pair<wstring, wstring> GetQuantityTime(SdaiInstance iQuantityInstance) const;

protected: // Methods

	void Load();
	void LoadUnits(SdaiInstance iProjectInstance);
};

#endif // IFCUNIT_H
