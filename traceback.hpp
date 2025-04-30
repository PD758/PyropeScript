/*
* Copyright 2025 github.com/PD758
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*      http://www.apache.org/licenses/LICENSE-2.0
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#pragma once

#include <type_traits>
using namespace std;

namespace _pyrope {
	struct TRACEBACK {
		size_t line;
		size_t column;
		const char* message;
	};

	struct _tag_traceback {};
	_tag_traceback TRACEBACK_ERROR;

	template<typename ReturnT>
	// Traceback error or return value
	struct RTRACEBACK {
		union {
			ReturnT value;
			TRACEBACK error;
		};
		bool is_traceback;

		RTRACEBACK(ReturnT v) : value(v), is_traceback(false) {}
		RTRACEBACK(TRACEBACK e, _tag_traceback) : error(e), is_traceback(true) {}
	};
	typedef RTRACEBACK<bool>
		NONE_OR_TRACEBACK;
}
using _pyrope::TRACEBACK, _pyrope::RTRACEBACK, _pyrope::TRACEBACK_ERROR, _pyrope::NONE_OR_TRACEBACK;

#include <iostream>

ostream& operator<<(ostream& os, _pyrope::TRACEBACK tb) {
	os << "Traceback(" << tb.message << " at line " << tb.line << " column " << tb.column << ')';
	return os;
}
