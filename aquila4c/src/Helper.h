#pragma once

#include <string>

bool strContains(const std::string& s1, const std::string& s2);

std::string strDropQuotes(const std::string& s);

bool strEndsWith(const std::string& s1, const std::string& s2);

bool strEquals(const std::string& s1, const std::string& s2);

bool strIsNumeric(const std::string& s);

void strReplace(std::string& str, const std::string& from, const std::string& to);

bool strStartsWith(const std::string& s1, const std::string& s2);
