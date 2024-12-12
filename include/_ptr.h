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
		assert(m_pTout != nullptr);
	}

	virtual ~_ptr()
	{
	}

	Tout* p()
	{
		return m_pTout;
	}

	operator Tout* ()
	{
		return m_pTout;
	}

	Tout* operator -> ()
	{
		return m_pTout;
	}
};