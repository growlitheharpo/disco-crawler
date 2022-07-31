/* Copyright (C) 2022 James Keats
 *
 * You may use, distribute, and modify this code under the terms of its modified
 * BSD-3-Clause license. Use for any commercial purposes is prohibited.
 * You should have received a copy of the license with this file. If not, please visit:
 * https://github.com/growlitheharpo/heart-engine-playground
 *
 */

#pragma once

#include "memory/managed_string.h"

#include <heart/types.h>

#include <heart/stl/type_traits.h>

#include <stdlib.h>
#include <string.h>

// Heavily, heavily SFINAE'd helper struct for parsing json.
// Can coerce from strings and populate int sizes smaller than what rapidjson supports.
template <typename TargetT>
struct GenericJsonReader
{
	template <typename T>
	struct CoerceFromStringImplInt
	{
		template <typename RapidjsonGenericMemberT>
		bool TryRead(T& outValue, RapidjsonGenericMemberT&& source)
		{
			if (!source.IsString())
				return false;

			auto str = source.GetString();
			if (!_strcmpi(str, "false"))
			{
				outValue = 0;
				return true;
			}
			else if (!_strcmpi(str, "true"))
			{
				outValue = 1;
				return true;
			}

			errno = 0;
			auto result = strtol(str, nullptr, 0);
			if (result == LONG_MAX || result == LONG_MIN || (result == 0 && errno == ERANGE))
				return false;

			outValue = T(result);
			return true;
		}
	};

	template <typename T>
	struct CoerceFromStringImplDecimal
	{
		template <typename RapidjsonGenericMemberT>
		bool TryRead(T& outValue, RapidjsonGenericMemberT&& source)
		{
			if (!source.IsString())
				return false;

			errno = 0;
			auto str = source.GetString();
			auto result = strtod(str, nullptr);
			if (result == 0.0 && errno == ERANGE)
				return false;

			outValue = result;
			return true;
		}
	};

	template <typename TargetT>
	struct CoerceFromStringHelper
	{
	};

	template <>
	struct CoerceFromStringHelper<bool> : public CoerceFromStringImplInt<bool>
	{
	};

	template <>
	struct CoerceFromStringHelper<int> : public CoerceFromStringImplInt<int>
	{
	};

	template <>
	struct CoerceFromStringHelper<unsigned int> : public CoerceFromStringImplInt<unsigned int>
	{
	};

	template <>
	struct CoerceFromStringHelper<int64> : public CoerceFromStringImplInt<int64>
	{
	};

	template <>
	struct CoerceFromStringHelper<uint64> : public CoerceFromStringImplInt<uint64>
	{
	};

	template <>
	struct CoerceFromStringHelper<double> : public CoerceFromStringImplDecimal<double>
	{
	};

	template <>
	struct CoerceFromStringHelper<float> : public CoerceFromStringImplDecimal<float>
	{
	};

	template <>
	struct CoerceFromStringHelper<const char*>
	{
		template <typename RapidjsonGenericMemberT>
		bool TryRead(const char* outValue, RapidjsonGenericMemberT&& source)
		{
			// A string should've already succeeded on the "first pass", we have
			// no need or ability to coerce it.
			return false;
		}
	};

	template <typename T>
	struct PromotionHelper
	{
		using ParsableType = T;
	};

	template <>
	struct PromotionHelper<int8>
	{
		using ParsableType = int;
	};

	template <>
	struct PromotionHelper<uint8>
	{
		using ParsableType = unsigned int;
	};

	template <>
	struct PromotionHelper<int16>
	{
		using ParsableType = int;
	};

	template <>
	struct PromotionHelper<uint16>
	{
		using ParsableType = unsigned int;
	};

	template <>
	struct PromotionHelper<ManagedString>
	{
		using ParsableType = const char*;
	};

	template <typename RapidJsonFieldT>
	bool AttemptRead(TargetT& outTarget, RapidJsonFieldT&& json)
	{
		using ParsableType = typename PromotionHelper<TargetT>::ParsableType;

		if (json.template Is<ParsableType>())
		{
			ParsableType value = json.template Get<ParsableType>();
			outTarget = TargetT(value);
			return true;
		}
		else if (json.IsString())
		{
			CoerceFromStringHelper<ParsableType> helper {};
			ParsableType value {};
			if (helper.TryRead(value, json))
			{
				outTarget = TargetT(value);
				return true;
			}
		}

		return false;
	}
};

template <typename TargetT, typename RapidjsonT>
bool ReadSingleField(TargetT& targetField, RapidjsonT&& jsonObj, const char* fieldName)
{
	if (auto iter = jsonObj.FindMember(fieldName); iter != jsonObj.MemberEnd())
	{
		GenericJsonReader<hrt::remove_cvref_t<TargetT>> reader {};
		return reader.AttemptRead(targetField, iter->value);
	}

	return false;
}

template <typename TargetT, typename RapidjsonT>
bool ReadFromFieldsArray(TargetT& targetField, RapidjsonT&& jsonObj, const char* fieldName)
{
	for (auto& entry : jsonObj)
	{
		const char* title;
		if (ReadSingleField(title, entry, "title") && _strcmpi(title, fieldName) == 0)
		{
			return ReadSingleField(targetField, entry, "value");
		}
	}

	return false;
}

#define READ_NAMED_SINGLE_FIELD(obj, field, json) ReadSingleField(obj.field, json, #field)

#define READ_NAMED_FROM_FIELDS_ARRAY(obj, field, arr) ReadFromFieldsArray(obj.field, arr, #field);
