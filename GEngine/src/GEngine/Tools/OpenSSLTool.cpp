#include "GEpch.h"
#include "OpenSSLTool.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace GEngine
{

	void OpenSSLTool::Init()
	{
		OpenSSL_add_all_algorithms();
		OpenSSL_add_all_ciphers();
		OpenSSL_add_all_digests();
	}

	std::string OpenSSLTool::ComputeMD5(const std::string& input)
	{
		MD5_CTX md5Context;
		MD5_Init(&md5Context);

		std::vector<unsigned char> data = StringToBinary(input);
		MD5_Update(&md5Context, data.data(), data.size());

		unsigned char digest[MD5_DIGEST_LENGTH];
		MD5_Final(digest, &md5Context);

		std::stringstream ss;
		for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
		{
			ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
		}
		return ss.str();
	}

	std::string OpenSSLTool::ComputeSHA256(const std::string& input)
	{
		SHA256_CTX sha256Context;
		SHA256_Init(&sha256Context);

		std::vector<unsigned char> data = StringToBinary(input);
		SHA256_Update(&sha256Context, data.data(), data.size());

		unsigned char digest[SHA256_DIGEST_LENGTH];
		SHA256_Final(digest, &sha256Context);

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