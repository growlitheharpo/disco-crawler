/* Copyright (C) 2022 James Keats
 *
 * You may use, distribute, and modify this code under the terms of its modified
 * BSD-3-Clause license. Use for any commercial purposes is prohibited.
 * You should have received a copy of the license with this file. If not, please visit:
 * https://github.com/growlitheharpo/heart-engine-playground
 *
 */

#include "memory/hash_lookup.h"

#include <heart/debug/assert.h>
#include <heart/hash/murmur.h>
#include <heart/scope_exit.h>

#include <set>

namespace
{
	bool ConvertToWide(const char* str, wchar_t* buffer, size_t bufferSize, bool toLower = true)
	{
		size_t requiredSize = 0;
		if (mbstowcs_s(&requiredSize, buffer, bufferSize, str, bufferSize - 1) != 0)
			return false;

		for (auto i = buffer; *i != 0; ++i)
			*i = towlower(*i);

		return true;
	}

	template <size_t N>
	bool ConvertToWide(const char* str, wchar_t (&X)[N], bool toLower = true)
	{
		return ConvertToWide(str, X, N, toLower);
	}

	std::wstring_view FindNextWord(std::wstring_view::iterator& position, std::wstring_view::iterator end)
	{
		auto b = position;
		while (b != end && !iswspace(*b) && !iswpunct(*b))
			++b;

		std::wstring_view result(position, b);

		while (b != end && (iswspace(*b) || iswpunct(*b)))
			++b;

		position = b;

		return result;
	}
}

void HashLookup::CrunchSingleString(const char* str, PoolIndex index)
{
	constexpr size_t SSOSize = 16 * Kilo;
	wchar_t buffer[SSOSize];

	if (!ConvertToWide(str, buffer))
		HEART_ASSERT(false);

	std::wstring_view wstr(buffer);
	auto iterator = wstr.begin();
	while (iterator != wstr.end())
	{
		auto word = FindNextWord(iterator, wstr.end());

		if (word.size() > 0)
		{
			auto hash = HeartMurmurHash3(word);
			m_lookup.emplace(hash, index);
		}
	}
}

hrt::vector<HashLookup::PoolIndex> HashLookup::LookupSingleWord(std::wstring_view word)
{
	hrt::vector<PoolIndex> result;
	HashType hash = HeartMurmurHash3(word);

	auto&& [begin, end] = m_lookup.equal_range(hash);
	while (begin != end)
	{
		result.push_back(begin->second);
		++begin;
	}

	return result;
}

uint32 HashLookup::Compile()
{
	auto& pool = ManagedStringPool::Get();
	if (!HEART_CHECK(pool.m_blob))
		return 0;

	uint8* bufferStart = pool.m_blob;
	uint8* bufferEnd = bufferStart + pool.m_size;
	PoolIndex poolIndex = 0;

	uint8* reader = bufferStart;
	while (reader < bufferEnd)
	{
		auto layout = (ManagedStringPool::StringLayout*)reader;
		auto str = &layout->firstCharacter;
		auto index = PoolIndex(reader - bufferStart);

		CrunchSingleString(str, index);

		auto len = strlen(str);
		reader = (uint8*)(str + len) + 1;
	}

	return uint32(m_lookup.size());
}

hrt::vector<ManagedString> HashLookup::LookupWord(const char* word)
{
	constexpr static size_t BufferSize = 4 * Kilo;
	wchar_t buffer[BufferSize];

	if (!ConvertToWide(word, buffer))
		return {};

	std::set<uint32> potentialMatches;

	std::wstring_view wstr(buffer);
	auto iterator = wstr.begin();
	while (iterator != wstr.end())
	{
		auto word = FindNextWord(iterator, wstr.end());

		if (word.size() > 0)
		{
			hrt::vector<uint32> matches = LookupSingleWord(word);
			potentialMatches.insert(matches.begin(), matches.end());
		}
	}

	hrt::vector<ManagedString> result;
	for (uint32 potentialIndex : potentialMatches)
	{
		const char* s = ManagedStringPool::Get().GetString(potentialIndex);

		if (strstr(s, word))
		{
			ManagedString& match = result.emplace_back();
			match.m_initialized = true;
			match.m_index = potentialIndex;
			match.m_size = uint16(strlen(s));
		}
	}

	return result;
}
