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
#include "types/object_type.h"
#include "json/read_helpers.h"

#include <heart/types.h>

#include <array>
#include <stdio.h>

struct Actor
{
	static constexpr ObjectType Type = ObjectType::Actor;

	uint32 id = 0;

	uint8 isPlayer = false;

	uint8 isNpc = false;

	uint8 isFemale = false;

	uint8 color = 0;

	uint64 articyId = 0;

	ManagedString name;

	ManagedString characterShortName;

	ManagedString pictures;

	ManagedString description;

	ManagedString shortDescription;

	ManagedString longDescription;

	std::array<ManagedString*, 6> GetStrings()
	{
		std::array<ManagedString*, 6> r = {
			&name,
			&characterShortName,
			&pictures,
			&description,
			&shortDescription,
			&longDescription};

		return r;
	}
};

template <typename RapidjsonT>
Actor ParseActor(RapidjsonT&& jsonObj)
{
	Actor result;

	auto fieldsArrayIter = jsonObj.FindMember("fields");
	if (fieldsArrayIter == jsonObj.MemberEnd() || !fieldsArrayIter->value.IsArray())
		return result;

	auto fieldsArray = fieldsArrayIter->value.GetArray();

	READ_NAMED_SINGLE_FIELD(result, id, jsonObj);

	READ_NAMED_FROM_FIELDS_ARRAY(result, isPlayer, fieldsArray);

	READ_NAMED_FROM_FIELDS_ARRAY(result, isNpc, fieldsArray);

	READ_NAMED_FROM_FIELDS_ARRAY(result, isFemale, fieldsArray);

	READ_NAMED_FROM_FIELDS_ARRAY(result, color, fieldsArray);

	ReadFromFieldsArray(result.articyId, fieldsArray, "Articy Id");

	READ_NAMED_FROM_FIELDS_ARRAY(result, name, fieldsArray);

	ReadFromFieldsArray(result.characterShortName, fieldsArray, "character_short_name");

	READ_NAMED_FROM_FIELDS_ARRAY(result, pictures, fieldsArray);

	READ_NAMED_FROM_FIELDS_ARRAY(result, description, fieldsArray);

	ReadFromFieldsArray(result.shortDescription, fieldsArray, "short_description");

	READ_NAMED_FROM_FIELDS_ARRAY(result, longDescription, fieldsArray);

	if (fieldsArray.Size() > 11)
	{
		printf("Character %s may have new and surprising fields.\n", result.name.CStr());
	}

	return result;
}
