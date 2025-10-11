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

	std::vector<std::string> StringHelper::Split(const std::string& string, std::vector<char> delimiters)
	{
		std::vector<std::string> result;
		std::string current;
		for (char c : string)
		{
			if (std::find(delimiters.begin(), delimiters.end(), c) != delimiters.end())
			{
				if (current.empty() == false)
				{
					result.push_back(current);
					current.clear();
				}
			}
			else
			{
				current += c;
			}
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
	std::vector<std::string> StringHelper::RemoveChar(const std::vector<std::string>& strings, char character)
	{
		std::vector<std::string> result;
		for (const auto& str : strings)
		{
			std::string newStr = str;
			newStr.erase(std::remove(newStr.begin(), newStr.end(), character), newStr.end());
			result.push_back(newStr);
		}
		return std::vector<std::string>();
	}
	std::vector<std::string> StringHelper::ClearEmptyStrings(const std::vector<std::string>& strings)
	{
		std::vector<std::string> result;
		for (const auto& str : strings)
		{
			if (str.empty() == false)
			{
				result.push_back(str);
			}
		}
		return result;
	}
}