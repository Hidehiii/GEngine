#pragma once

#include "GEngine/Core/Core.h"
#include "Log.h"

namespace GEngine
{
	// Contains a buffer of data
	// The buffer can be used to store data in memory
	struct Buffer
	{
		void* Data;
		uint64_t Size;

		Buffer()
			: Data(nullptr), Size(0)
		{
		}

		Buffer(const void* data, uint64_t size = 0)
			: Data((void*)data), Size(size)
		{
		}

		static Buffer Copy(const Buffer& other)
		{
			Buffer buffer;
			buffer.Allocate(other.Size);
			memcpy(buffer.Data, other.Data, other.Size);
			return buffer;
		}

		static Buffer Copy(const void* data, uint64_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}

		void Allocate(uint64_t size)
		{
			delete[](std::byte*)Data;
			Data = nullptr;

			if (size == 0)
				return;

			Data = new std::byte[size];
			Size = size;
		}

		void Release()
		{
			delete[](std::byte*)Data;
			Data = nullptr;
			Size = 0;
		}

		void ZeroInitialize()
		{
			if (Data)
				memset(Data, 0, Size);
		}

		template<typename T>
		T& Read(uint64_t offset = 0)
		{
			return *(T*)((std::byte*)Data + offset);
		}

		template<typename T>
		const T& Read(uint64_t offset = 0) const
		{
			return *(T*)((std::byte*)Data + offset);
		}

		std::byte* ReadBytes(uint64_t offset = 0) const
		{
			return (std::byte*)Data + offset;
		}

		void Write(const void* data, uint64_t size, uint64_t offset = 0)
		{
			GE_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
			memcpy((std::byte*)Data + offset, data, size);
		}

		operator bool() const
		{
			return Data;
		}

		std::byte& operator[](int index)
		{
			return ((std::byte*)Data)[index];
		}

		std::byte operator[](int index) const
		{
			return ((std::byte*)Data)[index];
		}

		template<typename T>
		T* As() const
		{
			return (T*)Data;
		}

		inline uint64_t GetSize() const { return Size; }
	};
}