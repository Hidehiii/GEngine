#pragma once
#include "GEngine/Core/Core.h"
#include "openssl/ssl.h"
#include "openssl/md5.h"
#include "openssl/sha.h"

namespace GEngine
{
	class GENGINE_API OpenSSLTool
	{
	public:
		OpenSSLTool() = delete;

		static void Init();

		static std::string ComputeMD5(const std::string& input);

		static std::string ComputeSHA256(const std::string& input);

		static std::vector<unsigned char> StringToBinary(const std::string& input);
		static std::string BinaryToString(const std::vector<unsigned char>& input);
	private:
		static MD5_CTX		m_MD5Context;
		static SHA256_CTX	m_SHA256Context;
	};
}


