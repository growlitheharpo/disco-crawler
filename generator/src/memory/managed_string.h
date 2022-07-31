/* Copyright (C) 2022 James Keats
 *
 * You may use, distribute, and modify this code under the terms of its modified
 * BSD-3-Clause license. Use for any commercial purposes is prohibited.
 * You should have received a copy of the license with this file. If not, please visit:
 * https://github.com/growlitheharpo/heart-engine-playground
 *
 */

#pragma once

#include "types/object_type.h"

#include <heart/copy_move_semantics.h>
#include <heart/debug/assert.h>
#include <heart/types.h>

#include <heart/stl/string.h>
#include <heart/stl/unordered_map.h>
#include <heart/stl/vector.h>

class ManagedStringPool;

class ManagedString
{
private:
	uint32 m_index = 0;
#if 0
	uint32 m_sizeAndLookback = 0;

	static constexpr uint32 BitsForType = 3;
	static constexpr uint32 BitsForSize = 10;
	static constexpr uint32 BitsForIndex = sizeof(m_sizeAndLookback) * 8 - (BitsForType + BitsForSize);

	static constexpr uint32 ShiftForType = (BitsForSize + BitsForIndex);
	static constexpr uint32 ShiftForSize = (BitsForIndex);
	static constexpr uint32 ShiftForIndex = 0;

	static constexpr uint32 MaskForType = 0b11100000000000000000000000000000;
	static constexpr uint32 MaskForSize = 0b00011111111110000000000000000000;
	static constexpr uint32 MaskForIndex = 0b0000000000000111111111111111111;

	template <typename T>
	T ReadPacked(uint32 bits, uint32 shift, uint32 mask) const
	{
		uint32 r = m_sizeAndLookback;
		r &= mask;
		r = r >> shift;
		return T(r);
	}

	template <typename T>
	void WritePacked(T val, uint32 bits, uint32 shift, uint32 mask)
	{
		HEART_ASSERT(val <= T((1 << bits) - 1));

		m_sizeAndLookback = m_sizeAndLookback & ~mask;
		m_sizeAndLookback = m_sizeAndLookback | ((uint32(val) << shift) & mask);
	}

public:
	uint32 GetSize() const
	{
		return ReadPacked<uint32>(BitsForSize, ShiftForSize, MaskForSize);
	}

	void SetSize(uint32 s)
	{
		WritePacked(s, BitsForSize, ShiftForSize, MaskForSize);
	}

	ObjectType GetLookbackType() const
	{
		return ReadPacked<ObjectType>(BitsForType, ShiftForType, MaskForType);
	}

	void SetLookbackType(ObjectType t)
	{
		WritePacked(t, BitsForType, ShiftForType, MaskForType);
	}

	uint32 GetLookbackIndex() const
	{
		return ReadPacked<uint32>(BitsForIndex, ShiftForIndex, MaskForIndex);
	}

	void SetLookbackIndex(size_t s)
	{
		return WritePacked(s, BitsForIndex, ShiftForIndex, MaskForIndex);
	}
#else
	uint32 m_size;
	ObjectType m_type;
	uint32 m_lookbackIndex;

public:
	uint32 GetSize() const
	{
		return m_size;
	}

	void SetSize(uint32 s)
	{
		m_size = s;
	}

	ObjectType GetLookbackType() const
	{
		return m_type;
	}

	void SetLookbackType(ObjectType t)
	{
		m_type = t;
	}

	uint32 GetLookbackIndex() const
	{
		return m_lookbackIndex;
	}

	void SetLookbackIndex(size_t s)
	{
		m_lookbackIndex = uint32(s);
	}
#endif

public:
	ManagedString() = default;
	~ManagedString() = default;

	ManagedString(const char* value);

	void InitializeLookback(ObjectType type, size_t index);

	const char* CStr() const;
};

class ManagedStringPool
{
public:
	static ManagedStringPool& Get();

private:
	struct Builder
	{
		typedef uint32 StringHash;
		typedef uint32 IndexIntoBlob;
		typedef size_t IndexIntoStorage;

		hrt::vector<hrt::string> storage;
		hrt::unordered_map<StringHash, IndexIntoBlob> previous;
		hrt::unordered_map<IndexIntoBlob, IndexIntoStorage> reverse;

		IndexIntoBlob runningSize = 0;

		IndexIntoBlob Push(const char* entry);
		const char* Lookup(IndexIntoBlob index) const;
		void Finalize(uint8*& outBlob, size_t& outSize);
	};

	uint8* m_blob = nullptr;
	size_t m_size = 0;

	Builder m_builder;

public:
	ManagedStringPool() = default;
	DISABLE_COPY_AND_MOVE_SEMANTICS(ManagedStringPool);
	~ManagedStringPool();

	void AddString(uint32& index, uint16& size, const char* str);
	const char* GetString(uint32 index) const;

	void FinalizeBuilder();
};
