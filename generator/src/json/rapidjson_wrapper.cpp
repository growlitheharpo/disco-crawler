/* Copyright (C) 2022 James Keats
 *
 * You may use, distribute, and modify this code under the terms of its modified
 * BSD-3-Clause license. Use for any commercial purposes is prohibited.
 * You should have received a copy of the license with this file. If not, please visit:
 * https://github.com/growlitheharpo/heart-engine-playground
 *
 */

#include "rapidjson_wrapper.h"

#include <heart/types.h>
#include <heart/debug/assert.h>

#include <rapidjson/filereadstream.h>

#include <cstdio>

rapidjson::Document ParseDocumentAsStream(const char* path)
{
	FILE* file = nullptr;
	fopen_s(&file, path, "r");

	if (!file)
		return rapidjson::Document {};

	constexpr size_t BufferSize = 1ull << 16;
	char* buffer = (char*)malloc(BufferSize);
	rapidjson::FileReadStream readStream(file, buffer, BufferSize);

	// Takes ~10 seconds in debug, ~1 second in release
	rapidjson::Document doc;
	doc.ParseStream(readStream);

	fclose(file);
	file = nullptr;

	return doc;
}
