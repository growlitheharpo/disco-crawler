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
	// The strings are in UTF8
	// Why
	template <typename IterT>
	int32 GetCodepoint(IterT& iter, IterT end)
	{
		// +0xxxxxxx
		constexpr int32 OneByteMask = 0b10000000;
		constexpr int32 OneByteMarker = 0;

		// +110xxxxx
		constexpr int32 TwoByteMask = 0b11100000;
		constexpr int32 TwoByteMarker = 0b11000000;

		// +1110xxxx
		constexpr int32 ThreeByteMask = 0b11110000;
		constexpr int32 ThreeByteMarker = 0b11100000;

		// +11110xxx
		constexpr int32 FourByteMask = 0b11111000;
		constexpr int32 FourByteMarker = 0b11110000;

		// +10xxxxxx but we want the x's
		constexpr int32 ExtraByteMask = 0b00111111;

		int32 codepoint = (int32(*iter) & 0xFF);
		int32 byteCount = 0;

		if ((codepoint & OneByteMask) == OneByteMarker)
		{
			codepoint = codepoint & ~OneByteMask;
			byteCount = 1;
		}
		else if ((codepoint & TwoByteMask) == TwoByteMarker)
		{
			codepoint = codepoint & ~TwoByteMask;
			byteCount = 2;
		}
		else if ((codepoint & ThreeByteMask) == ThreeByteMarker)
		{
			codepoint = codepoint & ~ThreeByteMask;
			byteCount = 3;
		}
		else if ((codepoint & FourByteMask) == FourByteMarker)
		{
			codepoint = codepoint & ~FourByteMask;
			byteCount = 4;
		}

		for (int i = 1; i < byteCount; ++i)
		{
			codepoint <<= 6;
			codepoint |= (*++iter) & ExtraByteMask;
		}

		return codepoint;
	}

	template <typename T, typename F>
	void ScanWhile(T& iterator, T end, F predicate)
	{
		while (iterator != end)
		{
			auto p = iterator;
			if (!predicate(GetCodepoint(iterator, end)))
			{
				iterator = p;
				break;
			}

			++iterator;
		}
	}

	template <typename T = std::string_view, typename IterT = typename T::iterator>
	T FindNextWord(IterT& position, IterT end)
	{
		auto begin = position;

		// Find the end of the current word
		ScanWhile(position, end, [](int32 codepoint) { return !iswspace(codepoint) && !iswpunct(codepoint); });

		// Create the result
		T result(begin, position);

		// Fast-forward to the start of the next word
		ScanWhile(position, end, [](int32 codepoint) { return iswspace(codepoint) || iswpunct(codepoint); });

		return result;
	}

	template <typename T = std::string_view>
	bool Contains(T haystack, T needle, bool insensitive = true)
	{
		if (needle.size() > haystack.size())
			return false;

		if (needle.size() < 1)
			return true;

		auto haystackSearchStop = haystack.end() - (needle.size() - 1);
		HEART_ASSERT(haystackSearchStop > haystack.begin() && haystackSearchStop <= haystack.end());
		for (auto haystackPos = haystack.begin(); haystackPos != haystackSearchStop; ++haystackPos)
		{
			auto haystackIter = haystackPos;
			auto needleIter = needle.begin();
			for (; needleIter != needle.end(); ++needleIter, ++haystackIter)
			{
				auto needleVal = GetCodepoint(needleIter, needle.end());
				auto haystackVal = GetCodepoint(haystackIter, haystack.end());

				bool match = needleVal == haystackVal;
				if (!match && insensitive)
					match = towlower(needleVal) == towlower(haystackVal);

				if (!match)
					break;
			}

			if (needleIter == needle.end())
				return true;

			GetCodepoint(haystackPos, haystack.end());
		}

		return false;
	}
}

void HashLookup::CrunchSingleString(const char* str, PoolIndex index)
{
	std::u8string_view view((char8_t*)str);
	auto iterator = view.begin();
	while (iterator != view.end())
	{
		auto word = FindNextWord<std::u8string_view>(iterator, view.end());
		if (word.size() > 0)
		{
			auto hash = HeartMurmurHash3(word);
			m_lookup.emplace(hash, index);
		}
	}
}

hrt::vector<HashLookup::PoolIndex> HashLookup::LookupSingleWord(std::u8string_view word)
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
	std::set<uint32> potentialMatches;
	hrt::vector<ManagedString> result;

	std::u8string_view str((char8_t*)word);
	auto iterator = str.begin();
	while (iterator != str.end())
	{
		auto word = FindNextWord<std::u8string_view>(iterator, str.end());

		if (word.size() > 0)
		{
			hrt::vector<uint32> matches = LookupSingleWord(word);
			potentialMatches.insert(matches.begin(), matches.end());
		}
	}

	for (uint32 potentialIndex : potentialMatches)
	{
		const char* s = ManagedStringPool::Get().GetString(potentialIndex);

		if (Contains(std::u8string_view((char8_t*)s), str))
		{
			ManagedString& match = result.emplace_back();
			match.m_initialized = true;
			match.m_index = potentialIndex;
			match.m_size = uint16(strlen(s));
		}
	}

	return result;
}
