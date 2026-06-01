#include "GEpch.h"
#include "OpenSSLTool.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace GEngine
{
	MD5_CTX		OpenSSLTool::m_MD5Context;
	SHA256_CTX	OpenSSLTool::m_SHA256Context;

	void OpenSSLTool::Init()
	{
		MD5_Init(&m_MD5Context);
		SHA256_Init(&m_SHA256Context);
	}

	std::string OpenSSLTool::ComputeMD5(const std::string& input)
	{
		std::vector<unsigned char> data = StringToBinary(input);
		MD5_Update(&m_MD5Context, data.data(), data.size());

		unsigned char digest[MD5_DIGEST_LENGTH];
		MD5_Final(digest, &m_MD5Context);

		std::stringstream ss;
		for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
		{
			ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
		}
		return ss.str();
	}

	std::string OpenSSLTool::ComputeSHA256(const std::string& input)
	{
		std::vector<unsigned char> data = StringToBinary(input);
		SHA256_Update(&m_SHA256Context, data.data(), data.size());

		unsigned char digest[SHA256_DIGEST_LENGTH];
		SHA256_Final(digest, &m_SHA256Context);

		std::stringstream ss;
		for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
		{
			ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
		}
		return ss.str();
	}

	std::vector<unsigned char> OpenSSLTool::StringToBinary(const std::string& input)
	{
		return std::vector<unsigned char>(input.begin(), input.end());
	}

	std::string OpenSSLTool::BinaryToString(const std::vector<unsigned char>& input)
	{
		return std::string(input.begin(), input.end());
	}
}