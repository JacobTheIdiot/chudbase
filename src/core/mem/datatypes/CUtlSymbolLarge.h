#pragma once
using CUtlSymLargeId = std::int32_t*;
class CUtlSymbolLarge
{
    inline static const CUtlSymLargeId UTL_INVALID_SYMBOL_LARGE = reinterpret_cast<CUtlSymLargeId>(-1);

public:
    CUtlSymbolLarge() { m_nID = UTL_INVALID_SYMBOL_LARGE; }
    CUtlSymbolLarge(CUtlSymbolLarge const& sym) { m_nID = sym.m_nID; }
    explicit CUtlSymbolLarge(CUtlSymLargeId id) { m_nID = id; }

    CUtlSymbolLarge& operator=(CUtlSymbolLarge const& src)
    {
        m_nID = src.m_nID;

        return *this;
    }

    bool operator==(CUtlSymbolLarge const& src) const { return m_nID == src.m_nID; }
    bool operator==(CUtlSymLargeId const& src) const { return m_nID == src; }
    bool operator!=(CUtlSymbolLarge const& src) const { return m_nID != src.m_nID; }
    bool operator!=(CUtlSymLargeId const& src) const { return m_nID != src; }
    explicit operator CUtlSymLargeId() const { return m_nID; }

    [[nodiscard]] const char* GetString() const
    {
        if (m_nID == UTL_INVALID_SYMBOL_LARGE)
            return "";

        return m_szString;
    }

    [[nodiscard]] bool IsValid() const
    {
        return m_nID != UTL_INVALID_SYMBOL_LARGE;
    }

private:
    union
    {
        CUtlSymLargeId m_nID;
        const char* m_szString;
    };
};

//// https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/mp/src/public/tier1/utlsymbollarge.h#L32-L114
//typedef intp UtlSymLargeId_t;
//#define UTL_INVAL_SYMBOL_LARGE  ((UtlSymLargeId_t)~0)
//class CUtlSymbolLarge
//{
//public:
//	// constructor, destructor
//	CUtlSymbolLarge()
//	{
//		u.m_Id = UTL_INVAL_SYMBOL_LARGE;
//	}
//
//	CUtlSymbolLarge(UtlSymLargeId_t id)
//	{
//		u.m_Id = id;
//	}
//	CUtlSymbolLarge(CUtlSymbolLarge const& sym)
//	{
//		u.m_Id = sym.u.m_Id;
//	}
//
//	// operator=
//	CUtlSymbolLarge& operator=(CUtlSymbolLarge const& src)
//	{
//		u.m_Id = src.u.m_Id;
//		return *this;
//	}
//
//	// operator==
//	bool operator==(CUtlSymbolLarge const& src) const
//	{
//		return u.m_Id == src.u.m_Id;
//	}
//
//	// operator==
//	bool operator==(UtlSymLargeId_t const& src) const
//	{
//		return u.m_Id == src;
//	}
//
//	// operator==
//	bool operator!=(CUtlSymbolLarge const& src) const
//	{
//		return u.m_Id != src.u.m_Id;
//	}
//
//	// operator==
//	bool operator!=(UtlSymLargeId_t const& src) const
//	{
//		return u.m_Id != src;
//	}
//
//	// Gets at the symbol
//	operator UtlSymLargeId_t const() const
//	{
//		return u.m_Id;
//	}
//
//	// Gets the string associated with the symbol
//	inline const char* String() const
//	{
//		if (u.m_Id == UTL_INVAL_SYMBOL_LARGE)
//			return "";
//		
//		return u.m_pAsString;
//	}
//
//	inline bool IsValid() const
//	{
//		return u.m_Id != UTL_INVAL_SYMBOL_LARGE ? true : false;
//	}
//
//private:
//	// Disallowed
//	CUtlSymbolLarge(const char* pStr);       // they need to go through the table to assign the ptr
//	bool operator==(const char* pStr) const; // disallow since we don't know if the table this is from was case sensitive or not... maybe we don't care
//
//	union
//	{
//		UtlSymLargeId_t m_Id;
//		char const* m_pAsString;
//	} u;
//};