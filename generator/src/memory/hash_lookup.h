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

#include <heart/stl/vector.h>

#include <map>
#include <string_view>

class HashLookup
{
	typedef uint32 HashType;
	typedef uint32 PoolIndex;

	std::multimap<HashType, PoolIndex> m_lookup;

	void CrunchSingleString(const char* str, PoolIndex index);

	hrt::vector<PoolIndex> LookupSingleWord(std::u8string_view word);

public:
	uint32 Compile();

	hrt::vector<ManagedString> LookupWord(const char* word);
};
