#ifndef IFCUNIT_H
#define IFCUNIT_H

#include "engine.h"
#include "stepengine.h"

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

// ------------------------------------------------------------------------------------------------
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

// ------------------------------------------------------------------------------------------------
class CIFCUnitProvider
{

private: // Members

	int64_t m_iModel;
	map<wstring, CIFCUnit*> m_mapUnits;

public: // Methods

	CIFCUnitProvider(int64_t iModel);
	virtual ~CIFCUnitProvider();

	const CIFCUnit* GetUnit(const wchar_t* szUnit) const;
	
	pair<wstring, wstring> GetQuantity(int_t iIFCQuantity, const char* szValueName, const wchar_t* szUnitName) const;
	pair<wstring, wstring> GetQuantityLength(int_t iIFCQuantity) const;
	pair<wstring, wstring> GetQuantityArea(int_t iIFCQuantity) const;
	pair<wstring, wstring> GetQuantityVolume(int_t iIFCQuantity) const;
	pair<wstring, wstring> GetQuantityCount(int_t iIFCQuantity) const;
	pair<wstring, wstring> GetQuantityWeight(int_t iIFCQuantity) const;
	pair<wstring, wstring> GetQuantityTime(int_t iIFCQuantity) const;

protected: // Methods

	void Load();
	void LoadUnits(int_t iIFCProjectInstance);
};

#endif // IFCUNIT_H
