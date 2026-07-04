#include "GEpch.h"
#include "ShaderCompiler.h"
#include "GEngine/Tools/StringHelper.h"
#include "GEngine/Graphics/Shader.h"
#include "GEngine/Graphics/Graphics.h"
#include <d3d12shader.h>
#include <SPIRVCross/spirv_cross.hpp>
namespace GEngine
{
	Ref<ShaderCompiler> ShaderCompiler::s_Instance = nullptr;
	
	namespace Utils
	{
		const std::string DXShaderTargetFromString(const std::string& target)
		{
			if (target == SHADER_STAGE_VERTEX)					return "vs_6_6";
			if (target == SHADER_STAGE_FRAGMENT)				return "ps_6_6";
			if (target == SHADER_STAGE_COMPUTE)					return "cs_6_6";
			if (target == SHADER_STAGE_HULL)					return "hs_6_6";
			if (target == SHADER_STAGE_DOMAIN)					return "ds_6_6";
			if (target == SHADER_STAGE_GEOMETRY)				return "gs_6_6";
			if (target == SHADER_STAGE_AMPLIFICATION)			return "as_6_6";
			if (target == SHADER_STAGE_MESH)					return "ms_6_6";
			GE_CORE_ASSERT(false, "Unknown shader target!");
			return "";
		}

		ShaderPropertyType ShaderPropertyTypeFromDxCBufferDesc(const D3D12_SHADER_VARIABLE_DESC& varDesc, const D3D12_SHADER_TYPE_DESC& typeDesc)
		{
			switch (typeDesc.Class)
			{
			case D3D_SVC_SCALAR:
			{
				switch (typeDesc.Type)
				{
				case D3D_SVT_INT:		return SHADER_PROPERTY_TYPE_INT;
				case D3D_SVT_FLOAT:		return SHADER_PROPERTY_TYPE_FLOAT;
				default:
					GE_CORE_ASSERT(false, "Unknown shader variable scalar type!");
				}
				break;
			}
			case D3D_SVC_VECTOR:
			{
				if (typeDesc.Type == D3D_SVT_FLOAT)
				{
					if (typeDesc.Columns == 2)		return SHADER_PROPERTY_TYPE_VECTOR_2;
					if (typeDesc.Columns == 3)		return SHADER_PROPERTY_TYPE_VECTOR_3;
					if (typeDesc.Columns == 4)		return SHADER_PROPERTY_TYPE_VECTOR_4;
				}
				GE_CORE_ASSERT(false, "Unknown shader variable vector type!");
				break;
			}
			case D3D_SVC_MATRIX_COLUMNS:
			{
				if (typeDesc.Type == D3D_SVT_FLOAT)
				{
					if (typeDesc.Rows == 2 && typeDesc.Columns == 2)		return SHADER_PROPERTY_TYPE_MATRIX_2X2;
					if (typeDesc.Rows == 2 && typeDesc.Columns == 3)		return SHADER_PROPERTY_TYPE_MATRIX_2X3;
					if (typeDesc.Rows == 2 && typeDesc.Columns == 4)		return SHADER_PROPERTY_TYPE_MATRIX_2X4;
					if (typeDesc.Rows == 3 && typeDesc.Columns == 2)		return SHADER_PROPERTY_TYPE_MATRIX_3X2;
					if (typeDesc.Rows == 3 && typeDesc.Columns == 3)		return SHADER_PROPERTY_TYPE_MATRIX_3X3;
					if (typeDesc.Rows == 3 && typeDesc.Columns == 4)		return SHADER_PROPERTY_TYPE_MATRIX_3X4;
					if (typeDesc.Rows == 4 && typeDesc.Columns == 2)		return SHADER_PROPERTY_TYPE_MATRIX_4X2;
					if (typeDesc.Rows == 4 && typeDesc.Columns == 3)		return SHADER_PROPERTY_TYPE_MATRIX_4X3;
					if (typeDesc.Rows == 4 && typeDesc.Columns == 4)		return SHADER_PROPERTY_TYPE_MATRIX_4X4;
				}
				GE_CORE_ASSERT(false, "Unknown shader variable matrix type!");
				break;
			}
			case D3D_SVC_STRUCT:
			{
				return SHADER_PROPERTY_TYPE_STRUCTURE;
			}
			}
		}

		ShaderPropertyType ShaderPropertyTypeFromDxResourceDesc(const D3D12_SHADER_INPUT_BIND_DESC& inputDesc)
		{
			switch (inputDesc.Type)
			{
			case D3D_SIT_CBUFFER:
				return SHADER_PROPERTY_TYPE_CBUFFER;
			case D3D_SIT_TEXTURE:
				return SHADER_PROPERTY_TYPE_TEXTURE_2D;
			case D3D_SIT_SAMPLER:
				return SHADER_PROPERTY_TYPE_SAMPLER;
			case D3D_SIT_UAV_RWTYPED:
				return SHADER_PROPERTY_TYPE_RWBUFFER;
			default:
				GE_CORE_ASSERT(false, "Unknown shader resource type!");
			}
		}

		ShaderPropertyType ShaderPropertyFromSpirvType(const spirv_cross::SPIRType& type, ShaderPropertyType baseType)
		{
			switch (type.basetype)
			{
			// base type bool
			case spirv_cross::SPIRType::BaseType::Boolean:
				return SHADER_PROPERTY_TYPE_BOOLEAN;
			// base type int
			case spirv_cross::SPIRType::BaseType::Int:
			{
				switch (type.columns)
				{
				case 1:
				{
					switch (type.vecsize)
					{
					case 1:		return SHADER_PROPERTY_TYPE_INT; // col 1, vec 1
					case 2:		return SHADER_PROPERTY_TYPE_INT_2; // col 1, vec 2
					case 3:		return SHADER_PROPERTY_TYPE_INT_3; // col 1, vec 3
					case 4:		return SHADER_PROPERTY_TYPE_INT_4; // col 1, vec 4
						default:
							GE_CORE_ASSERT(false, "Unknown shader variable int vector type!");
							break;
					}
					break;
				}
				case 2:
				{
					switch (type.vecsize)
					{
					case 2:		return SHADER_PROPERTY_TYPE_INT_2X2; // col 2, vec 2
					case 3:		return SHADER_PROPERTY_TYPE_INT_2X3; // col 2, vec 3
					case 4:		return SHADER_PROPERTY_TYPE_INT_2X4; // col 2, vec 4
						default:
							GE_CORE_ASSERT(false, "Unknown shader variable int matrix type!");
							break;
					}
					break;
				}
				case 3:
				{
					switch (type.vecsize)
					{
					case 2:		return SHADER_PROPERTY_TYPE_INT_3X2; // col 3, vec 2
					case 3:		return SHADER_PROPERTY_TYPE_INT_3X3; // col 3, vec 3
					case 4:		return SHADER_PROPERTY_TYPE_INT_3X4; // col 3, vec 4
						default:
							GE_CORE_ASSERT(false, "Unknown shader variable int matrix type!");
							break;
					}
					break;
				}
				case 4:
				{
					switch (type.vecsize)
					{
					case 2:		return SHADER_PROPERTY_TYPE_INT_4X2; // col 4, vec 2
					case 3:		return SHADER_PROPERTY_TYPE_INT_4X3; // col 4, vec 3
					case 4:		return SHADER_PROPERTY_TYPE_INT_4X4; // col 4, vec 4
						default:
							GE_CORE_ASSERT(false, "Unknown shader variable int matrix type!");
							break;
					}
					break;
				}
				default:
					GE_CORE_ASSERT(false, "Unknown shader variable int matrix type!");
					break;
				}
				break;
			}
			// base type float
			case spirv_cross::SPIRType::BaseType::Float:
			{
				switch (type.columns)
				{
				case 1:
				{
					switch (type.vecsize)
					{
					case 1:		return SHADER_PROPERTY_TYPE_FLOAT; // col 1, vec 1
					case 2:		return SHADER_PROPERTY_TYPE_VECTOR_2; // col 1, vec 2
					case 3:		return SHADER_PROPERTY_TYPE_VECTOR_3; // col 1, vec 3
					case 4:		return SHADER_PROPERTY_TYPE_VECTOR_4; // col 1, vec 4
						default:
							GE_CORE_ASSERT(false, "Unknown shader variable float vector type!");
							break;
					}
					break;
				}
				case 2:
				{
					switch (type.vecsize)
					{
					case 2:		return SHADER_PROPERTY_TYPE_MATRIX_2X2; // col 2, vec 2
					case 3:		return SHADER_PROPERTY_TYPE_MATRIX_2X3; // col 2, vec 3
					case 4:		return SHADER_PROPERTY_TYPE_MATRIX_2X4; // col 2, vec 4
						default:
							GE_CORE_ASSERT(false, "Unknown shader variable float matrix type!");
							break;
					}
					break;
				}
				case 3:
				{
					switch (type.vecsize)
					{
					case 2:		return SHADER_PROPERTY_TYPE_MATRIX_3X2; // col 3, vec 2
					case 3:		return SHADER_PROPERTY_TYPE_MATRIX_3X3; // col 3, vec 3
					case 4:		return SHADER_PROPERTY_TYPE_MATRIX_3X4; // col 3, vec 4
						default:
							GE_CORE_ASSERT(false, "Unknown shader variable float matrix type!");
							break;
					}
					break;
				}
				case 4:
				{
					switch (type.vecsize)
					{
					case 2:		return SHADER_PROPERTY_TYPE_MATRIX_4X2; // col 4, vec 2
					case 3:		return SHADER_PROPERTY_TYPE_MATRIX_4X3; // col 4, vec 3
					case 4:		return SHADER_PROPERTY_TYPE_MATRIX_4X4; // col 4, vec 4
						default:
							GE_CORE_ASSERT(false, "Unknown shader variable float matrix type!");
							break;
					}
					break;
				}
				default:
					GE_CORE_ASSERT(false, "Unknown shader variable float type!");
					break;
				}
				break;
			}
			// base type image
			case spirv_cross::SPIRType::BaseType::Image:
			{
				switch (type.image.dim)
				{
				case spv::Dim::Dim1D: 						return SHADER_PROPERTY_TYPE_TEXTURE_1D;
				case spv::Dim::Dim2D: 						return SHADER_PROPERTY_TYPE_TEXTURE_2D;
				case spv::Dim::Dim3D: 						return SHADER_PROPERTY_TYPE_TEXTURE_3D;
				case spv::Dim::DimCube: 					return SHADER_PROPERTY_TYPE_TEXTURE_CUBE;
				case spv::Dim::DimBuffer:
				{
					if (baseType == SHADER_PROPERTY_TYPE_STORAGE_IMAGE_UNKNOWN)
					{
						return SHADER_PROPERTY_TYPE_STORAGE_BUFFER;
					}
				}
				default:
					GE_CORE_ASSERT(false, "Unknown shader variable image type!");
				}
				GE_CORE_ASSERT(false, "Unknown shader variable image type!");
				break;
			}
			// base type sampler
			case spirv_cross::SPIRType::BaseType::Sampler:
				return SHADER_PROPERTY_TYPE_SAMPLER;
			// base type sampled image
			case spirv_cross::SPIRType::BaseType::SampledImage:
				return SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_2D;
			// base type storage buffer
			// base type struct
			case spirv_cross::SPIRType::BaseType::Struct:
			{
				switch (baseType)
				{
				case SHADER_PROPERTY_TYPE_UNIFORM_BUFFER_MENBER:	return SHADER_PROPERTY_TYPE_STRUCTURE;
				case SHADER_PROPERTY_TYPE_STORAGE_BUFFER:			return SHADER_PROPERTY_TYPE_STORAGE_BUFFER; // treat struct with storage buffer type as storage buffer
				}
				GE_CORE_ASSERT(false, "Unknown shader variable type!");
			}
			default:
				GE_CORE_ASSERT(false, "Unknown shader variable type!");
				return SHADER_PROPERTY_TYPE_NONE;
			}
		}

		uint32_t GetSpirvPropertyTypeArraySize(const spirv_cross::SPIRType& type)
		{
			uint32_t arraySize = 1;
			for (int i = 0; i < type.array.size(); i++)
			{
				if (type.array_size_literal[i])
				{
					arraySize *= type.array[i];
				}
				else
				{
					GE_CORE_ASSERT(false, "Shader variable array size is not a literal, which is currently not supported!");
				}
				GE_CORE_TRACE("Array dimension {}, size {}, literal {}!", i, type.array[i], type.array_size_literal[i]);
			}
			return arraySize;
		}
	}

	ShaderCompiler::ShaderCompiler()
	{
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_Utils));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_Compiler));
		m_Utils->CreateDefaultIncludeHandler(&m_IncludeHandler);
	}

	bool ShaderCompiler::Compile(const std::string& source, const std::string& target, const std::string& entryPoint, 
								std::vector<std::byte>& machineCode, std::vector<std::byte>& reflectionMeta)
	{
		DxcBuffer				sourceBuffer;
		sourceBuffer.Ptr		= source.data();
		sourceBuffer.Size		= source.size();
		sourceBuffer.Encoding	= DXC_CP_ACP;

		std::vector<LPCWSTR> args;
		if (Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL || Graphics::GetGraphicsAPI() == GRAPHICS_API_VULKAN)
		{
			args.push_back(L"-spirv");
			args.push_back(L"-fspv-reflect");
			args.push_back(L"-fspv-target-env=vulkan1.3");
		}
		args.push_back(L"-E");
		std::wstring entryPointW = StringHelper::StringToWideString(entryPoint);
		args.push_back(entryPointW.c_str());
		args.push_back(L"-T");
		std::wstring targetW = StringHelper::StringToWideString(Utils::DXShaderTargetFromString(target));
		args.push_back(targetW.c_str());
#if GE_DEBUG
		args.push_back(L"Zi");
#endif

		IDxcResult* result;
		HRESULT hr = m_Compiler->Compile(&sourceBuffer, args.data(), (uint32_t)args.size(), m_IncludeHandler, IID_PPV_ARGS(&result));
		if (FAILED(hr))
		{
			GE_CORE_ERROR("Shader compilation failed");
			return false;
		}
		IDxcBlobUtf8* errors;
		result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
		if (errors && errors->GetStringLength() != 0)
		{
			GE_CORE_ERROR("Shader compilation error: {0}", errors->GetStringPointer());
			return false;
		}

		IDxcBlob* compiledShaderBlob;
		hr = result->GetResult(&compiledShaderBlob);
		if (FAILED(hr))
		{
			GE_CORE_ERROR("Get compiled shader failed");
			return false;
		}

		machineCode.resize(compiledShaderBlob->GetBufferSize());
		memcpy(machineCode.data(), compiledShaderBlob->GetBufferPointer(), compiledShaderBlob->GetBufferSize());

		// copy reflection meta
		if (Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL || Graphics::GetGraphicsAPI() == GRAPHICS_API_VULKAN)
		{
			reflectionMeta.resize(compiledShaderBlob->GetBufferSize());
			memcpy(reflectionMeta.data(), compiledShaderBlob->GetBufferPointer(), compiledShaderBlob->GetBufferSize());
		}
		else if (Graphics::GetGraphicsAPI() == GRAPHICS_API_DIRECT3DX12)
		{
			IDxcBlob* reflectionBlob;
			hr = result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr);
			if (FAILED(hr) || reflectionBlob == nullptr)
			{
				GE_CORE_ERROR("Get shader reflection failed");
				return false;
			}
			reflectionMeta.resize(reflectionBlob->GetBufferSize());
			memcpy(reflectionMeta.data(), reflectionBlob->GetBufferPointer(), reflectionBlob->GetBufferSize());
		}
		

		return true;
	}
	Ref<ShaderCompiler> ShaderCompiler::Create()
	{
		s_Instance = CreateRef<ShaderCompiler>();
		return s_Instance;
	}
	Ref<ShaderCompiler> ShaderCompiler::Get()
	{
		return s_Instance;
	}
	void ShaderCompiler::Reflect(const std::vector<std::byte>& data, const std::string& target, ShaderReflectionInfo& reflectionOutput)
	{
		if (Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL || Graphics::GetGraphicsAPI() == GRAPHICS_API_VULKAN)
		{
			GE_CORE_ASSERT(data.size() % sizeof(uint32_t) == 0, "SPIR-V code size should be a multiple of {} bytes!", sizeof(uint32_t));
			std::vector<uint32_t> spirvCode(data.size() / sizeof(uint32_t));
			memcpy(spirvCode.data(), data.data(), data.size());
			ReflectSpirv(spirvCode, target, reflectionOutput);
		}
		else if (Graphics::GetGraphicsAPI() == GRAPHICS_API_DIRECT3DX12)
		{
			ReflectDxil((void*)data.data(), (uint32_t)data.size(), target, reflectionOutput);
		}
	}
	void ShaderCompiler::ReflectDxil(void* data, uint32_t length, const std::string& target, ShaderReflectionInfo& reflectionOutput)
	{
		ID3D12ShaderReflection* reflection;
		DxcBuffer reflectionData;
		reflectionData.Encoding = DXC_CP_ACP;
		reflectionData.Ptr = data;
		reflectionData.Size = length;
		m_Utils->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflection));

		//reflectionOutput

		D3D12_SHADER_DESC desc;
		reflection->GetDesc(&desc);
		GE_CORE_INFO("Shader stage {}", target);
		GE_CORE_INFO("Input parameters:");
		for (int i = 0; i < desc.InputParameters; i++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC inputDesc;
			reflection->GetInputParameterDesc(i, &inputDesc);
			GE_CORE_TRACE("Semantic name {}, index {}, register {}, SVtype {}, type {}!", inputDesc.SemanticName, inputDesc.SemanticIndex, inputDesc.Register, inputDesc.SystemValueType, inputDesc.ComponentType);
		}
		GE_CORE_INFO("Resource binding:");
		for (int i = 0; i < desc.BoundResources; i++)
		{
			D3D12_SHADER_INPUT_BIND_DESC resDesc;
			reflection->GetResourceBindingDesc(i, &resDesc);

			GE_CORE_TRACE("var name {}, type {}, bind point {}, bind count {}, register space {}!", resDesc.Name, resDesc.Type, resDesc.BindPoint, resDesc.BindCount, resDesc.Space);
			if(resDesc.Type == D3D_SIT_CBUFFER)
			{
				ID3D12ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByName(resDesc.Name);
				D3D12_SHADER_BUFFER_DESC bufferDesc;
				constantBuffer->GetDesc(&bufferDesc);
				GE_CORE_TRACE("  Constant buffer name {}, variables {}, size {}!", bufferDesc.Name, bufferDesc.Variables, bufferDesc.Size);
				for (int j = 0; j < bufferDesc.Variables; j++)
				{
					ID3D12ShaderReflectionVariable* variable = constantBuffer->GetVariableByIndex(j);
					ID3D12ShaderReflectionType* type = variable->GetType();
					D3D12_SHADER_VARIABLE_DESC varDesc;
					D3D12_SHADER_TYPE_DESC typeDesc;
					variable->GetDesc(&varDesc);
					type->GetDesc(&typeDesc);
					GE_CORE_TRACE("    var name {}, start offset {}, size {}!", varDesc.Name, varDesc.StartOffset, varDesc.Size);
					GE_CORE_TRACE("      type name {}, class {}, type {}, rows {}, columns {}, elements {}, members {}!", typeDesc.Name, typeDesc.Class, typeDesc.Type, typeDesc.Rows, typeDesc.Columns, typeDesc.Elements, typeDesc.Members);
				}
			}
		}
		GE_CORE_INFO("Output parameters:");
		for (int i = 0; i < desc.OutputParameters; i++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC outputDesc;
			reflection->GetOutputParameterDesc(i, &outputDesc);

			GE_CORE_TRACE("Semantic name {}, index {}, register {}, SVtype {}, type {}!", outputDesc.SemanticName, outputDesc.SemanticIndex, outputDesc.Register, outputDesc.SystemValueType, outputDesc.ComponentType);
		}
	}
	void ShaderCompiler::ReflectSpirv(const std::vector<uint32_t>& spirvCode, const std::string& target, ShaderReflectionInfo& reflectionOutput)
	{
		spirv_cross::Compiler			compiler(spirvCode);
		spirv_cross::ShaderResources	resources = compiler.get_shader_resources();

		//reflectionOutput

		GE_CORE_INFO("Shader stage {}", target);
		GE_CORE_INFO("Input parameters:");
		for (const auto& input : resources.stage_inputs)
		{
			auto& type = compiler.get_type(input.type_id);
			GE_CORE_TRACE("Name: {}, Location: {}, Type: {}", input.name, compiler.get_decoration(input.id, spv::DecorationLocation), type.basetype);
		}
		GE_CORE_INFO("Uniform buffers:");
		for (const auto& uniformBuffer : resources.uniform_buffers)
		{
			auto& type = compiler.get_type(uniformBuffer.type_id);

			ShaderReflectionCBufferInfo cbufferInfo;
			cbufferInfo.Name			= uniformBuffer.name;
			cbufferInfo.BindPoint		= compiler.get_decoration(uniformBuffer.id, spv::DecorationBinding);
			cbufferInfo.Size			= compiler.get_declared_struct_size(type);
			// ??? is that right?
			cbufferInfo.RegisterSpace	= compiler.get_decoration(uniformBuffer.id, spv::DecorationDescriptorSet);

			GE_CORE_TRACE("Name: {}, Binding: {}, Set: {}, Size: {}", uniformBuffer.name,
				compiler.get_decoration(uniformBuffer.id, spv::DecorationBinding),
				compiler.get_decoration(uniformBuffer.id, spv::DecorationDescriptorSet),
				compiler.get_declared_struct_size(type));
			// members of the uniform buffer
			for (int i = 0; i < type.member_types.size(); i++)
			{
				ShaderReflectionPropertyInfo	constantInfo;
				constantInfo.Name				= compiler.get_member_name(uniformBuffer.base_type_id, i);
				constantInfo.Type				= Utils::ShaderPropertyFromSpirvType(compiler.get_type(type.member_types[i]), SHADER_PROPERTY_TYPE_UNIFORM_BUFFER_MENBER);
				constantInfo.Size				= compiler.get_declared_struct_member_size(type, i);
				constantInfo.Offset				= compiler.get_member_decoration(uniformBuffer.base_type_id, i, spv::DecorationOffset);
				constantInfo.ArraySize			= Utils::GetSpirvPropertyTypeArraySize(compiler.get_type(type.member_types[i]));

				GE_CORE_TRACE("  Member Name: {}, Offset: {}, Type: {}, Width {}, Vec Size {}, Colums {}, Size {}, Array size {}", 
					constantInfo.Name,
					constantInfo.Offset,
					compiler.get_type(type.member_types[i]).basetype,
					compiler.get_type(type.member_types[i]).width,
					compiler.get_type(type.member_types[i]).vecsize,
					compiler.get_type(type.member_types[i]).columns,
					compiler.get_declared_struct_member_size(type, i),
					constantInfo.ArraySize);

				cbufferInfo.Properties.push_back(constantInfo);
			}

			reflectionOutput.CBuffers.insert(cbufferInfo);
		}
		GE_CORE_INFO("Separate images:");
		for (const auto& separateImage : resources.separate_images)
		{
			ShaderReflectionResourceInfo	imageInfo;
			imageInfo.Name					= separateImage.name;
			imageInfo.Type					= Utils::ShaderPropertyFromSpirvType(compiler.get_type(separateImage.type_id), SHADER_PROPERTY_TYPE_TEXTURE_UNKNOWN);
			imageInfo.BindPoint				= compiler.get_decoration(separateImage.id, spv::DecorationBinding);
			imageInfo.ArraySize				= Utils::GetSpirvPropertyTypeArraySize(compiler.get_type(separateImage.type_id));
			// ?? is that right?
			imageInfo.RegisterSpace			= compiler.get_decoration(separateImage.id, spv::DecorationDescriptorSet);

			auto& type = compiler.get_type(separateImage.type_id);
			GE_CORE_TRACE("Name: {}, Binding: {}, Set: {}, Type: {}, Dim {}, Depth {}, Format {}", 
				imageInfo.Name,
				imageInfo.BindPoint,
				compiler.get_decoration(separateImage.id, spv::DecorationDescriptorSet),
				type.basetype,
				type.image.dim,
				type.image.depth,
				type.image.format);

			reflectionOutput.Resources.insert(imageInfo);
		}
		GE_CORE_INFO("Separate samplers:");
		for (const auto& separateSampler : resources.separate_samplers)
		{
			ShaderReflectionResourceInfo	samplerInfo;
			samplerInfo.Name				= separateSampler.name;
			samplerInfo.Type				= Utils::ShaderPropertyFromSpirvType(compiler.get_type(separateSampler.type_id), SHADER_PROPERTY_TYPE_SAMPLER);
			samplerInfo.BindPoint			= compiler.get_decoration(separateSampler.id, spv::DecorationBinding);
			samplerInfo.ArraySize			= Utils::GetSpirvPropertyTypeArraySize(compiler.get_type(separateSampler.type_id));
			// ?? is that right?
			samplerInfo.RegisterSpace		= compiler.get_decoration(separateSampler.id, spv::DecorationDescriptorSet);

			auto& type = compiler.get_type(separateSampler.type_id);
			GE_CORE_TRACE("Name: {}, Binding: {}, Set: {}, Type: {}", separateSampler.name,
				compiler.get_decoration(separateSampler.id, spv::DecorationBinding),
				compiler.get_decoration(separateSampler.id, spv::DecorationDescriptorSet),
				type.basetype);

			reflectionOutput.Resources.insert(samplerInfo);
		}
		GE_CORE_INFO("Storage images:");
		for (const auto& storageImage : resources.storage_images)
		{
			ShaderReflectionResourceInfo	storageImageInfo;
			storageImageInfo.Name			= storageImage.name;
			storageImageInfo.Type			= Utils::ShaderPropertyFromSpirvType(compiler.get_type(storageImage.type_id), SHADER_PROPERTY_TYPE_STORAGE_IMAGE_UNKNOWN);
			storageImageInfo.BindPoint		= compiler.get_decoration(storageImage.id, spv::DecorationBinding);
			storageImageInfo.ArraySize		= Utils::GetSpirvPropertyTypeArraySize(compiler.get_type(storageImage.type_id));
			// ?? is that right?
			storageImageInfo.RegisterSpace	= compiler.get_decoration(storageImage.id, spv::DecorationDescriptorSet);

			auto& type = compiler.get_type(storageImage.type_id);
			GE_CORE_TRACE("Name: {}, Binding: {}, Set: {}, Type: {}, Dim {}, Depth {}, Format {}", 
				storageImage.name,
				compiler.get_decoration(storageImage.id, spv::DecorationBinding),
				compiler.get_decoration(storageImage.id, spv::DecorationDescriptorSet),
				type.basetype,
				type.image.dim,
				type.image.depth,
				type.image.format);

			reflectionOutput.Resources.insert(storageImageInfo);
		}
		GE_CORE_INFO("Storage buffers:");
		for (const auto& storageBuffer : resources.storage_buffers)
		{
			ShaderReflectionResourceInfo	bufferInfo;

			bufferInfo.Name					= storageBuffer.name;
			bufferInfo.Type					= Utils::ShaderPropertyFromSpirvType(compiler.get_type(storageBuffer.type_id), SHADER_PROPERTY_TYPE_STORAGE_BUFFER);
			bufferInfo.BindPoint			= compiler.get_decoration(storageBuffer.id, spv::DecorationBinding);
			bufferInfo.ArraySize			= Utils::GetSpirvPropertyTypeArraySize(compiler.get_type(storageBuffer.type_id));
			// ?? is that right?
			bufferInfo.RegisterSpace		= compiler.get_decoration(storageBuffer.id, spv::DecorationDescriptorSet);

			auto& type = compiler.get_type(storageBuffer.type_id);
			GE_CORE_TRACE("Name: {}, Binding: {}, Set: {}, Size: {}", storageBuffer.name,
				compiler.get_decoration(storageBuffer.id, spv::DecorationBinding),
				compiler.get_decoration(storageBuffer.id, spv::DecorationDescriptorSet),
				compiler.get_declared_struct_size(type));
			for (int i = 0; i < type.member_types.size(); i++)
			{
				auto& memberType = compiler.get_type(type.member_types[i]);
				std::string memberName = compiler.get_member_name(storageBuffer.base_type_id, i);
				GE_CORE_TRACE("  Member Name: {}, Offset: {}, Type: {}, Size {}", memberName,
					compiler.get_member_decoration(storageBuffer.base_type_id, i, spv::DecorationOffset),
					memberType.basetype,
					compiler.get_declared_struct_member_size(type, i));
			}

			reflectionOutput.Resources.insert(bufferInfo);
		}
		GE_CORE_INFO("Output parameters:");
		for (const auto& output : resources.stage_outputs)
		{
			auto& type = compiler.get_type(output.type_id);
			GE_CORE_TRACE("Name: {}, Location: {}, Type: {}", output.name, compiler.get_decoration(output.id, spv::DecorationLocation), type.basetype);
		}

	}
}