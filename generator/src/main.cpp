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

int main()
{
	// Takes ~10 seconds in debug, ~1 second in release
	rapidjson::Document doc = ParseDocumentAsStream("C:\\Users\\James\\Desktop\\DiscoDump\\Disco Elysium.json");

    return 0;
}
