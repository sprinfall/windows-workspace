#ifndef _TSTRING_H_
#define _TSTRING_H_

#include <tchar.h>
#include <ostream>

typedef std::basic_ostream<TCHAR> tostream;
typedef std::basic_istream<TCHAR> tistream;

class tstring {
public:
    tstring(const TCHAR *data = _T(""));
    tstring(const TCHAR *data, size_t offset, size_t len);
    tstring(const tstring &rhs);
    ~tstring();

    tstring& operator = (const tstring &rhs);

    TCHAR& operator[](size_t index);
    const TCHAR& operator[](size_t index) const;

    const TCHAR* c_str() const;

    size_t length() const;
    bool empty() const;

    tstring substr(size_t offset, size_t len = 0);
    tstring trim(TCHAR ch = _T(' '));

private:
    friend tostream& operator << (tostream& tos, const tstring& str);

    friend bool operator <  (const tstring& lhs, const tstring& rhs);
    friend bool operator >  (const tstring& lhs, const tstring& rhs);
    friend bool operator <= (const tstring& lhs, const tstring& rhs);
    friend bool operator >= (const tstring& lhs, const tstring& rhs);
    friend bool operator == (const tstring& lhs, const tstring& rhs);

private:
    TCHAR *data_;
};

tostream& operator << (tostream& tos, const tstring& str);

// ignore case
bool operator <  (const tstring& lhs, const tstring& rhs);
bool operator >  (const tstring& lhs, const tstring& rhs);
bool operator <= (const tstring& lhs, const tstring& rhs);
bool operator >= (const tstring& lhs, const tstring& rhs);
bool operator == (const tstring& lhs, const tstring& rhs);

#endif // _TSTRING_H_
