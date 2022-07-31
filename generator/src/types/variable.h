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
#include "json/read_helpers.h"
#include "types/object_type.h"

#include <heart/types.h>

#include <array>
#include <stdio.h>

struct Variable
{
	static constexpr ObjectType Type = ObjectType::Variable;

	int32 id;

	bool initialValueBool;

	int32 initialValueNumber;

	ManagedString name;

	ManagedString description;

	std::array<ManagedString*, 2> GetStrings()
	{
		std::array<ManagedString*, 2> r = {
			&name,
			&description,
		};

		return r;
	}
};

template <typename RapidjsonT>
Variable ParseVariable(RapidjsonT&& jsonObj)
{
	Variable result;

	auto fieldsArrayIter = jsonObj.FindMember("fields");
	if (fieldsArrayIter == jsonObj.MemberEnd() || !fieldsArrayIter->value.IsArray())
		return result;

	auto fieldsArray = fieldsArrayIter->value.GetArray();

	READ_NAMED_SINGLE_FIELD(result, id, jsonObj);

	READ_NAMED_FROM_FIELDS_ARRAY(result, name, fieldsArray);

	READ_NAMED_FROM_FIELDS_ARRAY(result, description, fieldsArray);

	bool isBool = ReadFromFieldsArray(result.initialValueBool, fieldsArray, "Initial Value");
	bool isNumber = ReadFromFieldsArray(result.initialValueNumber, fieldsArray, "Initial Value");
	if (!(isBool || isNumber))
	{
		printf("Failed to read initial value for variable %s\n", result.name.CStr());
	}

	if (fieldsArray.Size() > 3)
	{
		printf("Character %s may have new and surprising fields.\n", result.name.CStr());
	}

	return result;
}
