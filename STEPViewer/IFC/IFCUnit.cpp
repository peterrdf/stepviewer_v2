#include "stdafx.h"
#include "IFCUnit.h"

#include <assert.h>

// ------------------------------------------------------------------------------------------------
CIFCUnit::CIFCUnit(const wchar_t* szType, const wchar_t* szPrefix, const wchar_t* szName)
	: m_iType(UNKNOWN)
	, m_strType(L"")
    , m_strPrefix(L"")
    , m_strName(L"")
{
	ConvertType(szType);
	ConvertPrefix(szPrefix);
	ConvertName(szName);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CIFCUnit::~CIFCUnit()
{
}

// ------------------------------------------------------------------------------------------------
wstring CIFCUnit::GetType() const
{
    return m_strType;
}

// ------------------------------------------------------------------------------------------------
wstring CIFCUnit::GetPrefix() const
{
    return m_strPrefix;
}

// ------------------------------------------------------------------------------------------------
wstring CIFCUnit::GetName() const
{
    return m_strName;
}

// ------------------------------------------------------------------------------------------------
wstring CIFCUnit::GetUnit() const
{
    wstring strUnit = m_strPrefix;
    if (!strUnit.empty())
    {
        strUnit += L" ";
    }

    strUnit += m_strName;

    return strUnit;
}

// ------------------------------------------------------------------------------------------------
/*static*/ wstring CIFCUnit::GetPropertyValue(int64_t iIFCPropertySingleValue)
{
	wchar_t* szNominalValueADB = nullptr;
	sdaiGetAttrBN(iIFCPropertySingleValue, "NominalValue", sdaiUNICODE, &szNominalValueADB);

	if (szNominalValueADB == nullptr)
	{
		return L"";
	}

	wchar_t* szUnitADB = nullptr;
	sdaiGetAttrBN(iIFCPropertySingleValue, "Unit", sdaiUNICODE, &szUnitADB);

	wchar_t* szTypePath = (wchar_t *)sdaiGetADBTypePath(szNominalValueADB, 0);
	if (szTypePath == nullptr)
	{
		return szNominalValueADB;
	}

	if (wcscmp(szTypePath, L"IFCBOOLEAN") == 0)
	{
		return L"";
	}

	if (wcscmp(szTypePath, L"IFCIDENTIFIER") == 0)
	{
		return L"";
	}

	if (wcscmp(szTypePath, L"IFCINTEGER") == 0)
	{
		return L"";
	}

	if (wcscmp(szTypePath, L"IFCLABEL") == 0)
	{
		return L"";
	}

	if (wcscmp(szTypePath, L"IFCTEXT") == 0)
	{
		return L"";
	}

	if (wcscmp(szTypePath, L"IFCREAL") == 0)
	{
		return L"";
	}

	if (wcscmp(szTypePath, L"IFCCOUNTMEASURE") == 0)
	{
		return L"";
	}

	if (wcscmp(szTypePath, L"IFCPOSITIVERATIOMEASURE") == 0)
	{
		return L"";
	}

	if (wcscmp(szTypePath, L"IFCVOLUMETRICFLOWRATEMEASURE") == 0)
	{
		return L"";
	}

	return L"";
}

// ------------------------------------------------------------------------------------------------
void CIFCUnit::ConvertType(const wchar_t* szUnitType)
{
    assert(szUnitType != nullptr);

    if (wcscmp(szUnitType, L".ABSORBEDDOSEUNIT.") == 0)
    {
        m_iType = ABSORBEDDOSEUNIT;
        m_strType = L"ABSORBEDDOSEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".AREAUNIT.") == 0)
    {
        m_iType = AREAUNIT;
		m_strType = L"AREAUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".DOSEEQUIVALENTUNIT.") == 0)
    {
        m_iType = DOSEEQUIVALENTUNIT;
		m_strType = L"DOSEEQUIVALENTUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".ELECTRICCAPACITANCEUNIT.") == 0)
    {
        m_iType = ELECTRICCAPACITANCEUNIT;
		m_strType = L"ELECTRICCAPACITANCEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".ELECTRICCHARGEUNIT.") == 0)
    {
        m_iType = ELECTRICCHARGEUNIT;
		m_strType = L"ELECTRICCHARGEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".ELECTRICCONDUCTANCEUNIT.") == 0)
    {
        m_iType = ELECTRICCONDUCTANCEUNIT;
		m_strType = L"ELECTRICCONDUCTANCEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".ELECTRICCURRENTUNIT.") == 0)
    {
        m_iType = ELECTRICCURRENTUNIT;
		m_strType = L"ELECTRICCURRENTUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".ELECTRICRESISTANCEUNIT.") == 0)
    {
        m_iType = ELECTRICRESISTANCEUNIT;
		m_strType = L"ELECTRICRESISTANCEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".ELECTRICVOLTAGEUNIT.") == 0)
    {
        m_iType = ELECTRICVOLTAGEUNIT;
		m_strType = L"ELECTRICVOLTAGEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".ENERGYUNIT.") == 0)
    {
        m_iType = ENERGYUNIT;
		m_strType = L"ENERGYUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".FORCEUNIT.") == 0)
    {
        m_iType = FORCEUNIT;
		m_strType = L"FORCEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".FREQUENCYUNIT.") == 0)
    {
        m_iType = FREQUENCYUNIT;
		m_strType = L"FREQUENCYUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".ILLUMINANCEUNIT.") == 0)
    {
        m_iType = ILLUMINANCEUNIT;
		m_strType = L"ILLUMINANCEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".INDUCTANCEUNIT.") == 0)
    {
        m_iType = INDUCTANCEUNIT;
		m_strType = L"INDUCTANCEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".LENGTHUNIT.") == 0)
    {
        m_iType = LENGTHUNIT;
		m_strType = L"LENGTHUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".LUMINOUSFLUXUNIT.") == 0)
    {
        m_iType = LUMINOUSFLUXUNIT;
		m_strType = L"LUMINOUSFLUXUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".LUMINOUSINTENSITYUNIT.") == 0)
    {
        m_iType = LUMINOUSINTENSITYUNIT;
		m_strType = L"LUMINOUSINTENSITYUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".MAGNETICFLUXDENSITYUNIT.") == 0)
    {
        m_iType = MAGNETICFLUXDENSITYUNIT;
		m_strType = L"MAGNETICFLUXDENSITYUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".MAGNETICFLUXUNIT.") == 0)
    {
        m_iType = MAGNETICFLUXUNIT;
		m_strType = L"MAGNETICFLUXUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".MASSUNIT.") == 0)
    {
        m_iType = MASSUNIT;
		m_strType = L"MASSUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".PLANEANGLEUNIT.") == 0)
    {
        m_iType = PLANEANGLEUNIT;
		m_strType = L"PLANEANGLEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".POWERUNIT.") == 0)
    {
        m_iType = POWERUNIT;
		m_strType = L"POWERUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".PRESSUREUNIT.") == 0)
    {
        m_iType = PRESSUREUNIT;
		m_strType = L"PRESSUREUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".RADIOACTIVITYUNIT.") == 0)
    {
        m_iType = RADIOACTIVITYUNIT;
		m_strType = L"RADIOACTIVITYUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".SOLIDANGLEUNIT.") == 0)
    {
        m_iType = SOLIDANGLEUNIT;
		m_strType = L"SOLIDANGLEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".THERMODYNAMICTEMPERATUREUNIT.") == 0)
    {
        m_iType = THERMODYNAMICTEMPERATUREUNIT;
		m_strType = L"THERMODYNAMICTEMPERATUREUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".TIMEUNIT.") == 0)
    {
        m_iType = TIMEUNIT;
		m_strType = L"TIMEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".VOLUMEUNIT.") == 0)
    {
        m_iType = VOLUMEUNIT;
		m_strType = L"VOLUMEUNIT";
		return;
    }

    if (wcscmp(szUnitType, L".USERDEFINED.") == 0)
    {
        m_iType = USERDEFINED;
		m_strType = L"USERDEFINED";
		return;
    }

    m_iType = UNKNOWN;
	m_strType = L"UNKNOWN";
}

// ------------------------------------------------------------------------------------------------
void CIFCUnit::ConvertPrefix(const wchar_t* szPrefix)
{
    if (szPrefix == nullptr)
    {
        return;
    }

    if (wcscmp(szPrefix, L".EXA.") == 0)
    {
        m_strPrefix = L"Exa";
        return;
    }

    if (wcscmp(szPrefix, L".PETA.") == 0)
    {
        m_strPrefix = L"Peta";
        return;
    }

    if (wcscmp(szPrefix, L".TERA.") == 0)
    {
        m_strPrefix = L"Tera";
        return;
    }

    if (wcscmp(szPrefix, L".GIGA.") == 0)
    {
        m_strPrefix = L"Giga";
        return;
    }

    if (wcscmp(szPrefix, L".MEGA.") == 0)
    {
        m_strPrefix = L"Mega";
        return;
    }

    if (wcscmp(szPrefix, L".KILO.") == 0)
    {
        m_strPrefix = L"Kilo";
        return;
    }

    if (wcscmp(szPrefix, L".HECTO.") == 0)
    {
        m_strPrefix = L"Hecto";
        return;
    }

    if (wcscmp(szPrefix, L".DECA.") == 0)
    {
        m_strPrefix = L"Deca";
        return;
    }

    if (wcscmp(szPrefix, L".DECI.") == 0)
    {
        m_strPrefix = L"Deci";
        return;
    }

    if (wcscmp(szPrefix, L".CENTI.") == 0)
    {
        m_strPrefix = L"Centi";
        return;
    }

    if (wcscmp(szPrefix, L".MILLI.") == 0)
    {
        m_strPrefix = L"Milli";
        return;
    }

    if (wcscmp(szPrefix, L".MICRO.") == 0)
    {
        m_strPrefix = L"Micro";
        return;
    }

    if (wcscmp(szPrefix, L".NANO.") == 0)
    {
        m_strPrefix = L"Nano";
        return;
    }

    if (wcscmp(szPrefix, L".PICO.") == 0)
    {
        m_strPrefix = L"Pico";
        return;
    }

    if (wcscmp(szPrefix, L".FEMTO.") == 0)
    {
        m_strPrefix = L"Femto";
        return;
    }

    if (wcscmp(szPrefix, L".ATTO.") == 0)
    {
        m_strPrefix = L"Atto";
        return;
    }
}

// ------------------------------------------------------------------------------------------------
void CIFCUnit::ConvertName(const wchar_t* szName)
{
    assert(szName != nullptr);

    if (wcscmp(szName, L".AMPERE.") == 0)
    {
        m_strName = L"Ampere";
        return;
    }

    if (wcscmp(szName, L".BECQUEREL.") == 0)
    {
        m_strName = L"Becquerel";
        return;
    }

    if (wcscmp(szName, L".CANDELA.") == 0)
    {
        m_strName = L"Candela";
        return;
    }

    if (wcscmp(szName, L".COULOMB.") == 0)
    {
        m_strName = L"Coulomb";
        return;
    }

    if (wcscmp(szName, L".CUBIC_METRE.") == 0)
    {
        m_strName = L"Cubic Metre";
        return;
    }

    if (wcscmp(szName, L".DEGREE_CELSIUS.") == 0)
    {
        m_strName = L"Degree Celcius";
        return;
    }

    if (wcscmp(szName, L".FARAD.") == 0)
    {
        m_strName = L"Farad";
        return;
    }

    if (wcscmp(szName, L".GRAM.") == 0)
    {
        m_strName = L"Gram";
        return;
    }

    if (wcscmp(szName, L".GRAY.") == 0)
    {
        m_strName = L"Gray";
        return;
    }

    if (wcscmp(szName, L".HENRY.") == 0)
    {
        m_strName = L"Henry";
        return;
    }

    if (wcscmp(szName, L".HERTZ.") == 0)
    {
        m_strName = L"Hertz";
        return;
    }

    if (wcscmp(szName, L".JOULE.") == 0)
    {
        m_strName = L"Joule";
        return;
    }

    if (wcscmp(szName, L".KELVIN.") == 0)
    {
        m_strName = L"Kelvin";
        return;
    }

    if (wcscmp(szName, L".LUMEN.") == 0)
    {
        m_strName = L"Lumen";
        return;
    }

    if (wcscmp(szName, L".LUX.") == 0)
    {
        m_strName = L"Lux";
        return;
    }

    if (wcscmp(szName, L".METRE.") == 0)
    {
        m_strName = L"Metre";
        return;
    }

    if (wcscmp(szName, L".MOLE.") == 0)
    {
        m_strName = L"Mole";
        return;
    }

    if (wcscmp(szName, L".NEWTON.") == 0)
    {
        m_strName = L"Newton";
        return;
    }

    if (wcscmp(szName, L".OHM.") == 0)
    {
        m_strName = L"Ohm";
        return;
    }

    if (wcscmp(szName, L".PASCAL.") == 0)
    {
        m_strName = L"Pascal";
        return;
    }

    if (wcscmp(szName, L".RADIAN.") == 0)
    {
        m_strName = L"Radian";
        return;
    }

    if (wcscmp(szName, L".SECOND.") == 0)
    {
        m_strName = L"Second";
        return;
    }

    if (wcscmp(szName, L".SIEMENS.") == 0)
    {
        m_strName = L"Siemens";
        return;
    }

    if (wcscmp(szName, L".SIEVERT.") == 0)
    {
        m_strName = L"Sievert";
        return;
    }

    if (wcscmp(szName, L".SQUARE_METRE.") == 0)
    {
        m_strName = L"Square Metre";
        return;
    }

    if (wcscmp(szName, L".STERADIAN.") == 0)
    {
        m_strName = L"Steradian";
        return;
    }

    if (wcscmp(szName, L".TESLA.") == 0)
    {
        m_strName = L"Tesla";
        return;
    }

    if (wcscmp(szName, L".VOLT.") == 0)
    {
        m_strName = L"Volt";
        return;
    }

    if (wcscmp(szName, L".WATT.") == 0)
    {
        m_strName = L"Watt";
        return;
    }

    if (wcscmp(szName, L".WEBER.") == 0)
    {
        m_strName = L"Weber";
        return;
    }

    m_strName = szName;
}

// ------------------------------------------------------------------------------------------------
CIFCUnitProvider::CIFCUnitProvider(int64_t iModel)
    : m_iModel(iModel)
    , m_mapUnits()
{
    ASSERT(m_iModel != 0);

    Load();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CIFCUnitProvider::~CIFCUnitProvider()
{
    Clean();
}

// ------------------------------------------------------------------------------------------------
const CIFCUnit* CIFCUnitProvider::GetUnit(const wchar_t* szUnit) const
{
    auto itUnit = m_mapUnits.find(szUnit);
    if (itUnit != m_mapUnits.end())
    {
        return itUnit->second;
    }

    return nullptr;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCUnitProvider::Load()
{
    Clean();

    int64_t* iIFCProjectInstances = sdaiGetEntityExtentBN(m_iModel, (char*)"IFCPROJECT");

    int64_t iIFCProjectInstancesCount = sdaiGetMemberCount(iIFCProjectInstances);
    if (iIFCProjectInstancesCount > 0)
    {
        int64_t	iIFCProjectInstance = 0;
        if (iIFCProjectInstance != 0)
        {
            engiGetAggrElement(iIFCProjectInstances, 0, sdaiINSTANCE, &iIFCProjectInstance);

            LoadUnits(iIFCProjectInstance);
        }
    }
}

// ------------------------------------------------------------------------------------------------
void CIFCUnitProvider::LoadUnits(int_t iIFCProjectInstance)
{
    assert(iIFCProjectInstance != 0);

    int_t iIFCUnitAssignmentInstance = 0;
    sdaiGetAttrBN(iIFCProjectInstance, "UnitsInContext", sdaiINSTANCE, &iIFCUnitAssignmentInstance);

    const int_t ifcConversianBasedUnit_TYPE = sdaiGetEntity(m_iModel, "IFCCONVERSIONBASEDUNIT");
    const int_t ifcSIUnit_TYPE = sdaiGetEntity(m_iModel, "IFCSIUNIT");

    int_t* pUnitSet = nullptr;
    sdaiGetAttrBN(iIFCUnitAssignmentInstance, "Units", sdaiAGGR, &pUnitSet);

    int_t iUnitSetCount = sdaiGetMemberCount(pUnitSet);
    for (int_t iUnitSet = 0; iUnitSet < iUnitSetCount; ++iUnitSet)
    {
        int_t iIFCUnitInstance = 0;
        engiGetAggrElement(pUnitSet, iUnitSet, sdaiINSTANCE, &iIFCUnitInstance);

        if (sdaiGetInstanceType(iIFCUnitInstance) == ifcConversianBasedUnit_TYPE)
        {
            int_t iIFCMeasureWithUnitInstance = 0;
            sdaiGetAttrBN(iIFCUnitInstance, "ConversionFactor", sdaiINSTANCE, &iIFCMeasureWithUnitInstance);

            if (iIFCMeasureWithUnitInstance != 0)
            {
                int_t iIFCSIUnitInstance = 0;
                sdaiGetAttrBN(iIFCMeasureWithUnitInstance, "UnitComponent", sdaiINSTANCE, &iIFCSIUnitInstance);

                if (sdaiGetInstanceType(iIFCSIUnitInstance) == ifcSIUnit_TYPE)
                {
                    wchar_t* szUnitType = nullptr;
                    sdaiGetAttrBN(iIFCSIUnitInstance, "UnitType", sdaiUNICODE, &szUnitType);

                    wchar_t* szPrefix = nullptr;
                    sdaiGetAttrBN(iIFCSIUnitInstance, "Prefix", sdaiUNICODE, &szPrefix);

                    wchar_t* szName = nullptr;
                    sdaiGetAttrBN(iIFCSIUnitInstance, "Name", sdaiUNICODE, &szName);

                    CIFCUnit* pUnit = new CIFCUnit(szUnitType, szPrefix, szName);
                    m_mapUnits[pUnit->GetType()] = pUnit;
                }
                else
                {
                    assert(false);
                }
            } // if (iIFCMeasureWithUnitInstance != 0) 
            else
            {
                assert(false);
            }
        } // if (sdaiGetInstanceType(iIFCUnitInstance) == ifcConversianBasedUnit_TYPE)
        else
        {
            if (sdaiGetInstanceType(iIFCUnitInstance) == ifcSIUnit_TYPE)
            {
                wchar_t* szUnitType = nullptr;
                sdaiGetAttrBN(iIFCUnitInstance, "UnitType", sdaiUNICODE, &szUnitType);

                wchar_t* szPrefix = nullptr;
                sdaiGetAttrBN(iIFCUnitInstance, "Prefix", sdaiUNICODE, &szPrefix);

                wchar_t* szName = nullptr;
                sdaiGetAttrBN(iIFCUnitInstance, "Name", sdaiUNICODE, &szName);

                CIFCUnit* pUnit = new CIFCUnit(szUnitType, szPrefix, szName);
                m_mapUnits[pUnit->GetType()] = pUnit;
            } // if (sdaiGetInstanceType(iIFCUnitInstance) == ifcSIUnit_TYPE)
        } // else if (sdaiGetInstanceType(iIFCUnitInstance) == ifcConversianBasedUnit_TYPE)				
    } // for (int_t iUnitSet = 
}

// ------------------------------------------------------------------------------------------------
void CIFCUnitProvider::Clean()
{
    auto itUnit = m_mapUnits.begin();
    for (auto itUnit : m_mapUnits)
    {
        delete itUnit.second;
    }
    m_mapUnits.clear();
}
