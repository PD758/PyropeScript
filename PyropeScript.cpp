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

#include "allocator.hpp"

using namespace std;

int main() {

	Allocator alc;

	alc.reserve(10);

	auto& mem1 = alc.ialloc(4);
	++mem1;
	mem1.fill(0x21);
	mem1.hex(cout); cout << endl;
	mem1.fill(0x44);
	mem1.hex(cout); cout << endl;
	--mem1;
	
	return 0;
}