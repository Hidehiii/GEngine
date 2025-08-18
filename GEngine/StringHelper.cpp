#include "GEpch.h"
#include "StringHelper.h"

namespace GEngine
{
	std::string StringHelper::FromCharVector(const std::vector<char>& vector)
	{
		std::string str;
		for (char c : vector)
		{
			str += c;
		}
		return str;
	}

	std::string StringHelper::ToLower(std::string string)
	{
		std::transform(string.begin(), string.end(), string.begin(), ::tolower);
		return string;
	}

	std::string StringHelper::ToUpper(std::string string)
	{
		std::transform(string.begin(), string.end(), string.begin(), ::toupper);
		return string;
	}

	std::vector<std::string> StringHelper::Split(const std::string& string, char delimiter)
	{
		std::vector<std::string> result;
		std::stringstream ss(string);
		std::string item;
		while (std::getline(ss, item, delimiter))
		{
			item = item.substr(0, item.size());
			if (item.empty() == false)
				result.push_back(item);
		}
		return result;
	}

	std::string StringHelper::RemoveChar(const std::string& string, size_t pos, size_t len, char character)
	{
		std::string result = string;
		for (; pos < string.size() && len > 0; len--)
		{
			if (result[pos] == character)
			{
				result.erase(pos, 1);
			}
			else
			{
				pos++;
			}
		}
		return result;
	}
}