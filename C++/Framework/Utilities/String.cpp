#include "Framework.h"
#include "String.h"

#pragma warning(disable : 4996) //TODO : 추후 삭제!!!

void String::SplitString(vector<string>* result, string origin, string tok)
{
	result->clear();

	int cutAt = 0; //자를 위치
	while ((cutAt = (int)origin.find_first_of(tok)) != origin.npos)
	{
		if (cutAt > 0) //자르는 위치가 0보다 크면
			result->push_back(origin.substr(0, cutAt));

		origin = origin.substr(cutAt + 1);
	}

	if (origin.length() > 0) //자르고도 남은 것이 있다면
		result->push_back(origin.substr(0, cutAt));
}

void String::SplitString(vector<wstring>* result, wstring origin, wstring tok)
{
	result->clear();

	int cutAt = 0; //자를 위치s
	while ((cutAt = (int)origin.find_first_of(tok)) != origin.npos)
	{
		if (cutAt > 0) //자르는 위치가 0보다 크면
			result->push_back(origin.substr(0, cutAt));

		origin = origin.substr(cutAt + 1);
	}

	if (origin.length() > 0) //자르고도 남은것이 있다면
		result->push_back(origin.substr(0, cutAt));
}

bool String::StartsWith(string str, string comp)
{
	wstring::size_type index = str.find(comp);
	if (index != wstring::npos && (int)index == 0)
		return true;

	return false;
}

bool String::StartsWith(wstring str, wstring comp)
{
	wstring::size_type index = str.find(comp);
	if (index != wstring::npos && (int)index == 0)
		return true;

	return false;
}

bool String::Contain(string str, string comp)
{
	size_t found = str.find(comp);

	return found != wstring::npos;
}

bool String::Contain(wstring str, wstring comp)
{
	size_t found = str.find(comp);

	return found != wstring::npos;
}

void String::Replace(string * str, string comp, string rep)
{
	string temp = *str;

	size_t start_pos = 0;
	while ((start_pos = temp.find(comp, start_pos)) != wstring::npos)
	{
		temp.replace(start_pos, comp.length(), rep);
		start_pos += rep.length();
	}

	*str = temp;
}

void String::Replace(wstring * str, wstring comp, wstring rep)
{
	wstring temp = *str;

	size_t start_pos = 0;
	while ((start_pos = temp.find(comp, start_pos)) != wstring::npos)
	{
		temp.replace(start_pos, comp.length(), rep);
		start_pos += rep.length();
	}

	*str = temp;
}

wstring String::ToWString(string value)
{
	wstring temp = L"";
	temp.assign(value.begin(), value.end());

	return temp;
}

string String::ToString(wstring value)
{
	string temp = "";
	temp.assign(value.begin(), value.end());

	return temp;
}

string String::Format(const string format, ...)
{
	va_list args;

	va_start(args, format);
	size_t len = vsnprintf(NULL, 0, format.c_str(), args);
	va_end(args);

	vector<char> vec(len + 1);

	va_start(args, format);
	vsnprintf(&vec[0], len + 1, format.c_str(), args);
	va_end(args);

	return &vec[0];
}

wstring String::Format(const wstring format, ...)
{
	va_list args;

	va_start(args, format);
	size_t len = _vsnwprintf(NULL, 0, format.c_str(), args);
	va_end(args);

	vector<WCHAR> vec(len + 1);

	va_start(args, format);
	_vsnwprintf(&vec[0], len + 1, format.c_str(), args);
	va_end(args);

	return &vec[0];
}
