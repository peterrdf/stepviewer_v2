#include "stdafx.h"
#include "IFCProperty.h"
#include <assert.h>

// ------------------------------------------------------------------------------------------------
CIFCProperty::CIFCProperty(int64_t iInstance)
	: m_iInstance(iInstance)
	, m_strName(L"")
	, m_iType(-1)
{
	assert(m_iInstance != 0);

	/*
	* Name
	*/
	char * szPropertyName = NULL;
	GetNameOfProperty(m_iInstance, &szPropertyName);

	m_strName = CA2W(szPropertyName);

	/*
	* Parents
	*/
	int64_t iParentClassInstance = GetParentsByIterator(m_iInstance, 0);
	while (iParentClassInstance != 0)
	{
		char * szParentClassName = NULL;
		GetNameOfClass(iParentClassInstance, &szParentClassName);

		iParentClassInstance = GetParentsByIterator(m_iInstance, iParentClassInstance);
	}
}

// ------------------------------------------------------------------------------------------------
CIFCProperty::~CIFCProperty()
{
}

// ------------------------------------------------------------------------------------------------
// Getter
int64_t CIFCProperty::getInstance() const
{
	return m_iInstance;
}

// ------------------------------------------------------------------------------------------------
// Getter
const wchar_t * CIFCProperty::getName() const
{
	return m_strName.c_str();
}

// ------------------------------------------------------------------------------------------------
int64_t CIFCProperty::getType() const
{
	return m_iType;
}

// ------------------------------------------------------------------------------------------------
wstring CIFCProperty::getTypeAsString() const
{
	wstring strType = L"unknown";

	switch (getType())
	{
	case TYPE_OBJECTTYPE:
	{
		strType = L"owl:ObjectProperty";
	}
	break;

	case TYPE_BOOL_DATATYPE:
	{
		strType = L"owl:DatatypeProperty";
	}
	break;

	case TYPE_CHAR_DATATYPE:
	{
		strType = L"owl:DatatypeProperty";
	}
	break;

	case TYPE_DOUBLE_DATATYPE:
	{
		strType = L"owl:DatatypeProperty";
	}
	break;

	case TYPE_INT_DATATYPE:
	{
		strType = L"owl:DatatypeProperty";
	}
	break;

	default:
	{
		assert(false);
	}
	break;
	} // switch (getType())

	return strType;
}

// ------------------------------------------------------------------------------------------------
wstring CIFCProperty::getRange() const
{
	wstring strRange = L"unknown";

	switch (getType())
	{
	case TYPE_OBJECTTYPE:
	{
		strRange = L"xsd:object";
	}
	break;

	case TYPE_BOOL_DATATYPE:
	{
		strRange = L"xsd:boolean";
	}
	break;

	case TYPE_CHAR_DATATYPE:
	{
		strRange = L"xsd:string";
	}
	break;

	case TYPE_DOUBLE_DATATYPE:
	{
		strRange = L"xsd:double";
	}
	break;

	case TYPE_INT_DATATYPE:
	{
		strRange = L"xsd:integer";
	}
	break;

	default:
	{
		assert(false);
	}
	break;
	} // switch (getType())

	return strRange;
}

// ------------------------------------------------------------------------------------------------
wstring CIFCProperty::getCardinality(int64_t iInstance) const
{
	ASSERT(iInstance != 0);

	wchar_t szBuffer[100];

	int64_t iCard = 0;

	switch (getType())
	{
		case TYPE_OBJECTTYPE:
		{
			int64_t * piInstances = NULL;
			GetObjectTypeProperty(iInstance, getInstance(), &piInstances, &iCard);
		}
		break;

		case TYPE_BOOL_DATATYPE:
		{
			bool ** bValue = NULL;
			GetDataTypeProperty(iInstance, getInstance(), (void **)&bValue, &iCard);
		}
		break;

		case TYPE_CHAR_DATATYPE:
		{
			char ** szValue = NULL;
			GetDataTypeProperty(iInstance, getInstance(), (void **)&szValue, &iCard);
		}
		break;

		case TYPE_DOUBLE_DATATYPE:
		{
			double * pdValue = NULL;
			GetDataTypeProperty(iInstance, getInstance(), (void **)&pdValue, &iCard);
		}
		break;

		case TYPE_INT_DATATYPE:
		{
			int64_t * piValue = NULL;
			GetDataTypeProperty(iInstance, getInstance(), (void **)&piValue, &iCard);
		}
		break;

		default:
		{
			assert(false);
		}
		break;
	} // switch (getType())

	int64_t	iMinCard = 0;
	int64_t iMaxCard = 0;
	GetRestrictions(iInstance, iMinCard, iMaxCard);

	if ((iMinCard == -1) && (iMaxCard == -1))
	{
		swprintf(szBuffer, 100, L"%lld of [0 - infinity>", iCard);
	}
	else
	{
		if (iMaxCard == -1)
		{
			swprintf(szBuffer, 100, L"%lld of [%lld - infinity>", iCard, iMinCard);
		}
		else
		{
			swprintf(szBuffer, 100, L"%lld of [%lld - %lld]", iCard, iMinCard, iMaxCard);
		}
	}

	return szBuffer;
}

// ------------------------------------------------------------------------------------------------
void CIFCProperty::GetRestrictions(int64_t iInstance, int64_t & iMinCard, int64_t & iMaxCard) const
{
	assert(iInstance != 0);

	int64_t iClassInstance = GetInstanceClass(iInstance);
	assert(iClassInstance != 0);

	iMinCard = 0;
	iMaxCard = 0;
	GetPropertyRestrictions(iClassInstance, getInstance(), &iMinCard, &iMaxCard);
}
