template<typename Tout>
class _ptr
{

private: // Members

	Tout* m_pTout;

public: // Methods

	template<typename Tin>
	_ptr(Tin* pTin)
		: m_pTout(nullptr)
	{
		assert(pTin != nullptr);

		m_pTout = dynamic_cast<Tout*>(pTin);
	}

	virtual ~_ptr()
	{
	}

	Tout* operator -> ()
	{
		return m_pTout;
	}
};