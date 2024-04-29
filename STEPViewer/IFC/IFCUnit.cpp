#include "stdafx.h"
#include "IFCUnit.h"

// ************************************************************************************************
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

/*virtual*/ CIFCUnit::~CIFCUnit()
{}

// ------------------------------------------------------------------------------------------------
wstring CIFCUnit::GetType() const
{
    return m_strType;
}

wstring CIFCUnit::GetPrefix() const
{
    return m_strPrefix;
}

wstring CIFCUnit::GetName() const
{
    return m_strName;
}

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

void CIFCUnit::ConvertType(const wchar_t* szUnitType)
{
    ASSERT(szUnitType != nullptr);

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

void CIFCUnit::ConvertName(const wchar_t* szName)
{
    ASSERT(szName != nullptr);

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

// ************************************************************************************************
CIFCUnitProvider::CIFCUnitProvider(SdaiModel iModel)
    : m_iModel(iModel)
    , m_mapUnits()
{
    ASSERT(m_iModel != 0);

    Load();
}

/*virtual*/ CIFCUnitProvider::~CIFCUnitProvider()
{
    for (auto itUnit : m_mapUnits)
    {
        delete itUnit.second;
    }
}

const CIFCUnit* CIFCUnitProvider::GetUnit(const wchar_t* szUnit) const
{
    auto itUnit = m_mapUnits.find(szUnit);
    if (itUnit != m_mapUnits.end())
    {
        return itUnit->second;
    }

    return nullptr;
}

pair<wstring, wstring> CIFCUnitProvider::GetQuantity(SdaiInstance iQuantityInstance, const char* szValueName, const wchar_t* szUnitName) const
{
    wchar_t* szQuantityName = nullptr;
    sdaiGetAttrBN(iQuantityInstance, "Name", sdaiUNICODE, &szQuantityName);

    wchar_t* szQuantityDescription = nullptr;
    sdaiGetAttrBN(iQuantityInstance, "Description", sdaiUNICODE, &szQuantityDescription);

    wchar_t* szValue = nullptr;
    sdaiGetAttrBN(iQuantityInstance, szValueName, sdaiUNICODE, &szValue);

    wchar_t* szUnit = nullptr;
    sdaiGetAttrBN(iQuantityInstance, "Unit", sdaiUNICODE, &szUnit);

    wstring strName = szQuantityName;
    wstring strValue = szValue != nullptr ? szValue : L"NA";

    if (szUnit != nullptr)
    {
        strValue += L" ";
        strValue += szUnit;
    } // if (szUnit != nullptr)
    else
    {
        auto itUnit = m_mapUnits.find(szUnitName);
        if (itUnit != m_mapUnits.end())
        {
            strValue += L" ";
            strValue += itUnit->second->GetName();
        }
    } // else if (szUnit != nullptr)	

    if ((szQuantityDescription != nullptr) && (wcslen(szQuantityDescription) > 0))
    {
        strValue += L" ('";
        strValue += szQuantityDescription;
        strValue += L"')";
    }

    return pair<wstring, wstring>(strName, strValue);
}

pair<wstring, wstring> CIFCUnitProvider::GetQuantityLength(SdaiInstance iQuantityInstance) const
{
    return GetQuantity(iQuantityInstance, "LengthValue", L"LENGTHUNIT");
}

pair<wstring, wstring> CIFCUnitProvider::GetQuantityArea(SdaiInstance iQuantityInstance) const
{
    return GetQuantity(iQuantityInstance, "AreaValue", L"AREAUNIT");
}

pair<wstring, wstring> CIFCUnitProvider::GetQuantityVolume(SdaiInstance iQuantityInstance) const
{
    return GetQuantity(iQuantityInstance, "VolumeValue", L"VOLUMEUNIT");
}

pair<wstring, wstring> CIFCUnitProvider::GetQuantityCount(SdaiInstance iQuantityInstance) const
{
    return GetQuantity(iQuantityInstance, "CountValue", L"");
}

pair<wstring, wstring> CIFCUnitProvider::GetQuantityWeight(SdaiInstance iQuantityInstance) const
{
    return GetQuantity(iQuantityInstance, "WeigthValue", L"MASSUNIT");
}

pair<wstring, wstring> CIFCUnitProvider::GetQuantityTime(SdaiInstance iQuantityInstance) const
{
    return GetQuantity(iQuantityInstance, "TimeValue", L"TIMEUNIT");
}

void CIFCUnitProvider::Load()
{
    SdaiAggr pProjectAggr = sdaiGetEntityExtentBN(m_iModel, (char*)"IFCPROJECT");

    SdaiInteger iMembersCount = sdaiGetMemberCount(pProjectAggr);
    if (iMembersCount > 0)
    {
        SdaiInstance iProjectInstance = 0;
        engiGetAggrElement(pProjectAggr, 0, sdaiINSTANCE, &iProjectInstance);
        
        if (iProjectInstance != 0)
        {
            LoadUnits(iProjectInstance);
        }
    }
}

void CIFCUnitProvider::LoadUnits(SdaiInstance iProjectInstance)
{
    ASSERT(iProjectInstance != 0);

    SdaiInstance iUnitsInContextInstance = 0;
    sdaiGetAttrBN(iProjectInstance, "UnitsInContext", sdaiINSTANCE, &iUnitsInContextInstance);

    SdaiAggr pUnits = nullptr;
    sdaiGetAttrBN(iUnitsInContextInstance, "Units", sdaiAGGR, &pUnits);

    SdaiInteger iMembersCount = sdaiGetMemberCount(pUnits);
    for (SdaiInteger iMember = 0; iMember < iMembersCount; iMember++)
    {
        SdaiInstance iAggrInstance = 0;
        engiGetAggrElement(pUnits, iMember, sdaiINSTANCE, &iAggrInstance);

        if (sdaiGetInstanceType(iAggrInstance) == sdaiGetEntity(m_iModel, "IFCCONVERSIONBASEDUNIT"))
        {
            SdaiInstance iConversionFactorInstance = 0;
            sdaiGetAttrBN(iAggrInstance, "ConversionFactor", sdaiINSTANCE, &iConversionFactorInstance);

            if (iConversionFactorInstance != 0)
            {
                SdaiInstance iUnitComponentInstance = 0;
                sdaiGetAttrBN(iConversionFactorInstance, "UnitComponent", sdaiINSTANCE, &iUnitComponentInstance);

                if (sdaiGetInstanceType(iUnitComponentInstance) == sdaiGetEntity(m_iModel, "IFCSIUNIT"))
                {
                    wchar_t* szUnitType = nullptr;
                    sdaiGetAttrBN(iUnitComponentInstance, "UnitType", sdaiUNICODE, &szUnitType);

                    wchar_t* szPrefix = nullptr;
                    sdaiGetAttrBN(iUnitComponentInstance, "Prefix", sdaiUNICODE, &szPrefix);

                    wchar_t* szName = nullptr;
                    sdaiGetAttrBN(iUnitComponentInstance, "Name", sdaiUNICODE, &szName);

                    CIFCUnit* pUnit = new CIFCUnit(szUnitType, szPrefix, szName);
                    m_mapUnits[pUnit->GetType()] = pUnit;
                }
                else
                {
                    ASSERT(FALSE);
                }
            } // if (iConversionFactorInstance != 0) 
            else
            {
                ASSERT(FALSE);
            }
        } // IFCCONVERSIONBASEDUNIT
        else
        {
            if (sdaiGetInstanceType(iAggrInstance) == sdaiGetEntity(m_iModel, "IFCSIUNIT"))
            {
                wchar_t* szUnitType = nullptr;
                sdaiGetAttrBN(iAggrInstance, "UnitType", sdaiUNICODE, &szUnitType);

                wchar_t* szPrefix = nullptr;
                sdaiGetAttrBN(iAggrInstance, "Prefix", sdaiUNICODE, &szPrefix);

                wchar_t* szName = nullptr;
                sdaiGetAttrBN(iAggrInstance, "Name", sdaiUNICODE, &szName);

                auto pUnit = new CIFCUnit(szUnitType, szPrefix, szName);
                m_mapUnits[pUnit->GetType()] = pUnit;
            } // IFCSIUNIT
        } // else IFCCONVERSIONBASEDUNIT
    } // for (SdaiInteger iMember = ...
}

