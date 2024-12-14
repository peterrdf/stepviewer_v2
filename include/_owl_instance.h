#include "_owl_geometry.h"

// ************************************************************************************************
class _owl_instance : public _instance
{

public:  // Methods

	_owl_instance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix)
		: _instance(iID, pGeometry, pTransformationMatrix)
	{
	}

	virtual ~_owl_instance()
	{
	}

public: // Properties

	OwlInstance getOwlInstance() const { return getGeometryAs<_owl_geometry>()->getOwlInstance(); }

	wstring getName() const
	{
		CString strName;
		strName.Format(_T("#%lld"), getOwlInstance());

		strName += L" ";
		strName += getClassName();

		return (LPCWSTR)strName;
	}

	static wstring getName(OwlInstance owlInstance)
	{
		CString strName;
		strName.Format(_T("#%lld"), owlInstance);

		strName += L" ";
		strName += getClassName(owlInstance);

		return (LPCWSTR)strName;
	}

	OwlClass getClassInstance() const
	{
		return getClassInstance(getOwlInstance());
	}

	static OwlClass getClassInstance(OwlInstance owlInstance)
	{
		return GetInstanceClass(owlInstance);
	}

	const wchar_t* getClassName() const
	{
		return getClassName(getOwlInstance());
	}

	static const wchar_t* getClassName(OwlInstance owlInstance)
	{
		wchar_t* szClassName = nullptr;
		GetNameOfClassW(GetInstanceClass(owlInstance), &szClassName);

		return szClassName;
	}
};