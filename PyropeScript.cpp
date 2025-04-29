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
#include <iostream>
#include <string>
#include <vector>

#include "allocator.hpp"
#include "tokenizer.hpp"

using namespace std;

int main() {

	while (true) {

		string source, line = "START";

		cout << "Source code:\nvvvvvvvvvv" << endl;

		while (getline(cin, line) && line != "END") {
			source += line + '\n';
		}

		cout << "^^^^^^^^^^^^\n" << endl;

		vector<Token> tokens;
		NONE_OR_TRACEBACK res = tokenize(source, tokens);
		if (res.is_traceback) {
			cout << res.error << endl;
		}

		for (Token token : tokens) {
			cout << token << endl;
		}

		cout << "\n\n";

	}
	return 0;
}