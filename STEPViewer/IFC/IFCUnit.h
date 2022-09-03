#ifndef IFCUNIT_H
#define IFCUNIT_H

#include "engine.h"
#include "ifcengine.h"

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

// ----------------------------------------------------------------------------
class CIFCUnit
{

public: // Methods

	// ------------------------------------------------------------------------
	// ctor
	CIFCUnit(const wchar_t * szType, const wchar_t * szPrefix, const wchar_t * szName);

	// ------------------------------------------------------------------------
	// dtor
    virtual ~CIFCUnit();

	// ------------------------------------------------------------------------
    // Getter
    wstring getPrefix() const;

	// ------------------------------------------------------------------------
    // Getter
    wstring getName() const;

	// ------------------------------------------------------------------------
    // PREFIX + SPACE + NAME
    wstring getUnit() const;

	// ------------------------------------------------------------------------
    // Helper
	static void LoadUnits(int_t iIFCModel, int_t iIFCProjectInstance, map<wstring, CIFCUnit *> & mapUnits);

	// ------------------------------------------------------------------------
	// Helper
	static wstring GetPropertyValue(int64_t iIFCPropertySingleValue);

protected: // Methods

	// ------------------------------------------------------------------------
    // Helper
    void ConvertType(const wchar_t * szUnitType);

	// ------------------------------------------------------------------------
    // Helper
	void ConvertPrefix(const wchar_t * szPrefix);

	// ------------------------------------------------------------------------
    // Helper
	void ConvertName(const wchar_t * szName);

private: // Members	

	// ------------------------------------------------------------------------
	// Type
    int m_iType;

	// ------------------------------------------------------------------------
	// Prefix
	wstring m_strType;

	// ------------------------------------------------------------------------
	// Prefix
    wstring m_strPrefix;

	// ------------------------------------------------------------------------
	// Name
    wstring m_strName;
};

#endif // IFCUNIT_H
