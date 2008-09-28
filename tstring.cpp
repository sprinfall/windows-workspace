#include "tstring.h"

tstring::tstring(const TCHAR *data)
{
    size_t size = _tcslen(data) + 1;
    data_ = new TCHAR[size];
    if (size > 1) {
        _tcscpy_s(data_, size, data);
    } else {
        data_[0] = _T('\0');
    }
}

tstring::tstring(const TCHAR *data, size_t offset, size_t len)
{
    size_t size = _tcslen(data);
    if (offset >= size) { offset = size - 1; }
    if (len == 0 || len > size - offset) { len = size - offset; }

    data_ = new TCHAR[len + 1];
    _tcsncpy_s(data_, len + 1, data + offset, len);
}

tstring::tstring(const tstring &rhs)
{
    size_t size = _tcslen(rhs.data_) + 1;
    data_ = new TCHAR[size];
    _tcscpy_s(data_, size , rhs.data_);
}


tstring::~tstring()
{
    delete []data_;
}

tstring& tstring::operator = (const tstring &rhs)
{
    if (this != &rhs) {
        delete []data_;
        size_t size = _tcslen(rhs.data_) + 1;
        data_ = new TCHAR[size];
        _tcscpy_s(data_, size , rhs.data_);
    }
    return *this;
}

TCHAR& tstring::operator[](size_t index)
{
    if (index >= length()) { return data_[0]; }
    return data_[index];
}

const TCHAR& tstring::operator[](size_t index) const
{
    if (index >= length()) { return data_[0]; }
    return data_[index];
}

const TCHAR* tstring::c_str() const
{
    return const_cast<const TCHAR *>(data_);
}

size_t tstring::length() const
{
    return _tcslen(data_);
}

bool tstring::empty() const
{
    return length() == 0;
}

tstring tstring::substr(size_t offset, size_t len)
{
    return tstring(data_, offset, len);
}

tstring tstring::trim(TCHAR ch)
{
    if (empty()) { return _T(""); }

    size_t i = 0, j = length();
    while (i < j && data_[i] == ch) { ++i; }
    while (i < j-1 && data_[j-1] == ch) { --j; }

    return substr(i, j - i);
}


tostream& operator << (tostream& tos, const tstring& str)
{
    return tos << str.data_;
}

// ignore case

bool operator <  (const tstring& lhs, const tstring& rhs)
{
    return _tcsicmp(lhs.data_, rhs.data_) < 0;
}
bool operator >  (const tstring& lhs, const tstring& rhs)
{
    return _tcsicmp(lhs.data_, rhs.data_) > 0;
}
bool operator <= (const tstring& lhs, const tstring& rhs)
{
    return _tcsicmp(lhs.data_, rhs.data_) <= 0;
}
bool operator >= (const tstring& lhs, const tstring& rhs)
{
    return _tcsicmp(lhs.data_, rhs.data_) >= 0;
}
bool operator == (const tstring& lhs, const tstring& rhs)
{
    return _tcsicmp(lhs.data_, rhs.data_) == 0;
}
