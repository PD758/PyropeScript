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

#include <iostream>

#include <string>
#include <vector>
#include <unordered_map>
#include <stack>

#include "traceback.hpp"

using namespace std;

namespace _pyrope {
	enum class TokenType {
		Type,			// NONE, CHAR, UCHAR, INT{2,4,8,16,32}, UINT{2,4,8,16,32}, INT, UINT, FLOAT, DOUBLE, STRING, USTRING, LIST
		Keyword,		// IF, WHILE, FOR, IMPORT, RETURN, FUNCTION
		Identifier,		// variable name, etc.
		LiteralString,  // "string"
		LiteralChar,	// 'c'
		LiteralNumber,  // 1234567890
		LiteralFloat,   // 123.456
		LiteralBool,	// True/False
		Operator,		// +, -, *, /, //, %, **, >, >=, <, <=, |, ^, &, ==, !=, ||, &&, !
		Assignment,		// =, &=, +=, -=, *=, /=, %=, //=, **=
		Punctuator,		// ; : , [ ] ( ) .
		Follow,			// ->
		INDENT,
		DEDENT,
		NEWLINE,
		END_OF_FILE,
		UNKNOWN,
	};

	struct Token {
		TokenType type;
		string lexeme;
		size_t line = 1;
		size_t column = 1;
	};

	inline void addToken(vector<Token>& tokens, TokenType type,
				const string& lexeme, size_t line, size_t column) {
		tokens.push_back({ type, lexeme, line, column });
	}

	TokenType handleKeyword(const string& lexeme) {
		static const unordered_map<string, TokenType> keywords = {
		// Types
			{"NONE",	TokenType::Type},
			{"CHAR",	TokenType::Type},
			{"UCHAR",	TokenType::Type},
			{"INT2",	TokenType::Type},
			{"UINT2",	TokenType::Type},
			{"INT4",	TokenType::Type},
			{"UINT4",	TokenType::Type},
			{"INT8",	TokenType::Type},
			{"UINT8",	TokenType::Type},
			{"INT16",	TokenType::Type},
			{"UINT16",	TokenType::Type},
			{"INT32",	TokenType::Type},
			{"UINT32",	TokenType::Type},
			{"INT",		TokenType::Type},
			{"UINT",	TokenType::Type},
			{"FLOAT",	TokenType::Type},
			{"DOUBLE",	TokenType::Type},
			{"STRING",	TokenType::Type},
			{"USTRING",	TokenType::Type},
			{"LIST",	TokenType::Type},
		// Keywords
			{"IF",		TokenType::Keyword},
			{"WHILE",	TokenType::Keyword},
			{"FOR",		TokenType::Keyword},
			{"IMPORT",	TokenType::Keyword},
			{"RETURN",	TokenType::Keyword},
			{"FUNCTION",TokenType::Keyword},
		// Literals
			{"True",	TokenType::LiteralBool},
			{"False",	TokenType::LiteralBool}
		};
		auto it = keywords.find(lexeme);
		if (it != keywords.end())
			return it->second;
		return TokenType::Identifier;
	}

	NONE_OR_TRACEBACK tokenize(string& source, vector<Token>& tokens) {
		size_t current = 0;
		size_t line = 1;
		stack<size_t> indentStack;
		indentStack.push(0);

		bool handle_LF = true;
		size_t line_start = 0;

		while (current < source.length()) {
			size_t tok_start = current;
			size_t column = current - line_start + 1;
			char c = source[current];

			if (handle_LF) {
				size_t curr_indent = 0;
				// handle indents
				while (current < source.length() && source[current] == ' ') {
					curr_indent++;
					current++;
				}

				// skip empty lines and comments
				if (current < source.length() &&
					(source[current] == '\n' || source[current] == '#')) {
					if (source[current] == '\n') {
						line++;
						line_start = current++;
					}
					else {
						while (current < source.length() && source[current] != '\n') {
							current++;
						}
						if (current < source.length() && source[current] == '\n') {
							line++;
							line_start = current++;
						}
					}
					handle_LF = true;
					continue;
				}

				// Check indents
				if (curr_indent > indentStack.top()) {
					indentStack.push(curr_indent);
					addToken(tokens, TokenType::INDENT, "", line, column);
				}
				else {
					while (curr_indent < indentStack.top()) {
						indentStack.pop();
						addToken(tokens, TokenType::DEDENT, "", line, column);
					}
					if (curr_indent != indentStack.top()) {
						addToken(tokens, TokenType::UNKNOWN, source.substr(tok_start, current - tok_start), line, column);
						return NONE_OR_TRACEBACK({ line, column, "IndentationError: unindent does not match any outer indentation level" }, TRACEBACK_ERROR);
					}
				}
				handle_LF = false;
				if (current >= source.length())
					break;

				tok_start = current;
				column = current - line_start + 1;
				c = source[current];
			}

			// skip spaces and comments
			if (isspace(c) && c != '\n') {
				current++;
				continue;
			}
			if (c == '#') {
				while (current < source.length() && source[current] != '\n') {
					current++;
				}
				continue;
			}
			if (c == '\n') {
				addToken(tokens, TokenType::NEWLINE, "\\n", line, column);
				current++;
				line++;
				line_start = current;
				handle_LF = true;
				continue;
			}

			// recognize tokens
			// indentifiers & keywords
			if (isalpha(c) || c == '_') {
				while (current < source.length() &&
					(isalnum(source[current]) || source[current] == '_')) {
					current++;
				}
				string lexeme = source.substr(tok_start, current - tok_start);
				TokenType type = handleKeyword(lexeme);
				addToken(tokens, type, lexeme, line, column);
				continue;
			}

			// number literals
			if (isdigit(c)) {
				// TODO: hex, octal, binary
				while (current < source.length() &&
					isdigit(source[current])) {
					current++;
				}
				if (current < source.length() && source[current] == '.') {
					if (current + 1 < source.length()
						&& isdigit(source[current + 1])) {
						// float literal
						current++;
						while (current < source.length() && isdigit(source[current])) {
							current++;
						}
						string lexeme = source.substr(tok_start, current - tok_start);
						addToken(tokens, TokenType::LiteralFloat, lexeme, line, column);
						continue;
					}
					else {
						// this may be .method() call
					}
				}
				string lexeme = source.substr(tok_start, current - tok_start);
				addToken(tokens, TokenType::LiteralNumber, lexeme, line, column);
				continue;
			}

			// string literals
			if (c == '"') {
				current++;
				tok_start++;
				while (current < source.length() && source[current] != '"') {
					if (source[current] == '\\') {
						current++;
						if (current > source.length()) break;
					}
					if (source[current] == '\n') {
						addToken(tokens, TokenType::UNKNOWN, source.substr(tok_start, current - tok_start), line, column);
						return NONE_OR_TRACEBACK({ line, column, "SyntaxError: Unterminated string literal" }, TRACEBACK_ERROR);
					}
					current++;
				}
				if (current >= source.length() || source[current] != '"') {
					addToken(tokens, TokenType::UNKNOWN, source.substr(tok_start, current - tok_start), line, column);
					return NONE_OR_TRACEBACK({ line, column, "SyntaxError: Unterminated string literal" }, TRACEBACK_ERROR);
				}
				string lexeme = source.substr(tok_start, current - tok_start);
				current++;
				addToken(tokens, TokenType::LiteralString, lexeme, line, column);
				continue;
			}

			// char literals
			if (c == '\'') {
				current++;
				tok_start++;
				char char_val = '\0';
				if (current < source.length()) {
					if (source[current] == '\\') {
						current++;
						if (current < source.length()) {
							switch (source[current]) {
							case 'n':
								char_val = '\n'; break;
							case 't':
								char_val = '\t'; break;
							case '\\':
								char_val = '\\'; break;
							case '\'':
								char_val = '\''; break;
							case 'r':
								char_val = '\r'; break;
							default:
								char_val = source[current];
								break;
							}
							current++;
						}
						else {
							addToken(tokens, TokenType::UNKNOWN, source.substr(tok_start, current - tok_start), line, column);
							return NONE_OR_TRACEBACK({ line, column, "SyntaxError: Unterminated char escape sequence" }, TRACEBACK_ERROR);
						}
					}
					else if (source[current] != '\'') {
						char_val = source[current];
						current++;
					}
					else {
						addToken(tokens, TokenType::UNKNOWN, source.substr(tok_start, current - tok_start), line, column);
						return NONE_OR_TRACEBACK({ line, column, "SyntaxError: Empty char literal" }, TRACEBACK_ERROR);
					}
				}
				else {
					addToken(tokens, TokenType::UNKNOWN, source.substr(tok_start, current - tok_start), line, column);
					return NONE_OR_TRACEBACK({ line, column, "SyntaxError: Unterminated char literal" }, TRACEBACK_ERROR);
				}

				if (current < source.length() && source[current] == '\'') {
					current++;
					string lexeme(1, char_val);
					addToken(tokens, TokenType::LiteralChar, lexeme, line, column);
				}
				else {
					addToken(tokens, TokenType::UNKNOWN, source.substr(tok_start, current - tok_start), line, column);
					return NONE_OR_TRACEBACK({ line, column, "SyntaxError: Char literal must contain only one single character" }, TRACEBACK_ERROR);
				}
				continue;
			}

			// check operators
			if (source.substr(current, 3) == "//=") {
				addToken(tokens, TokenType::Assignment, "//=", line, column); current += 3; continue;
			}
			if (source.substr(current, 3) == "**=") {
				addToken(tokens, TokenType::Assignment, "**=", line, column); current += 3; continue;
			}
			if (source.substr(current, 2) == "//") {
				addToken(tokens, TokenType::Operator, "//", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "**") {
				addToken(tokens, TokenType::Operator, "**", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "&=") {
				addToken(tokens, TokenType::Assignment, "&=", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "+=") {
				addToken(tokens, TokenType::Assignment, "+=", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "-=") {
				addToken(tokens, TokenType::Assignment, "-=", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "*=") {
				addToken(tokens, TokenType::Assignment, "*=", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "/=") {
				addToken(tokens, TokenType::Assignment, "/=", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "%=") {
				addToken(tokens, TokenType::Assignment, "%=", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "->") {
				addToken(tokens, TokenType::Follow, "->", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "==") {
				addToken(tokens, TokenType::Operator, "==", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "!=") {
				addToken(tokens, TokenType::Operator, "!=", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "&&") {
				addToken(tokens, TokenType::Operator, "&&", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "||") {
				addToken(tokens, TokenType::Operator, "||", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == ">=") {
				addToken(tokens, TokenType::Operator, ">=", line, column); current += 2; continue;
			}
			if (source.substr(current, 2) == "<=") {
				addToken(tokens, TokenType::Operator, "<=", line, column); current += 2; continue;
			}
			switch (c) {
			case '+':
				addToken(tokens, TokenType::Operator, "+", line, column); current++; continue;
			case '-':
				addToken(tokens, TokenType::Operator, "-", line, column); current++; continue;
			case '*':
				addToken(tokens, TokenType::Operator, "*", line, column); current++; continue;
			case '/':
				addToken(tokens, TokenType::Operator, "/", line, column); current++; continue;
			case '%':
				addToken(tokens, TokenType::Operator, "%", line, column); current++; continue;
			case '=':
				addToken(tokens, TokenType::Assignment, "=", line, column); current++; continue;
			case ';':
				addToken(tokens, TokenType::Punctuator, ";", line, column); current++; continue;
			case ':':
				addToken(tokens, TokenType::Punctuator, ":", line, column); current++; continue;
			case '.':
				addToken(tokens, TokenType::Punctuator, ".", line, column); current++; continue;
			case ',':
				addToken(tokens, TokenType::Punctuator, ",", line, column); current++; continue;
			case '[':
				addToken(tokens, TokenType::Punctuator, "[", line, column); current++; continue;
			case ']':
				addToken(tokens, TokenType::Punctuator, "]", line, column); current++; continue;
			case '(':
				addToken(tokens, TokenType::Punctuator, "(", line, column); current++; continue;
			case ')':
				addToken(tokens, TokenType::Punctuator, ")", line, column); current++; continue;
			case '>':
				addToken(tokens, TokenType::Operator, ">", line, column); current++; continue;
			case '<':
				addToken(tokens, TokenType::Operator, "<", line, column); current++; continue;
			case '&':
				addToken(tokens, TokenType::Operator, "&", line, column); current++; continue;
			case '^':
				addToken(tokens, TokenType::Operator, "^", line, column); current++; continue;
			case '|':
				addToken(tokens, TokenType::Operator, "|", line, column); current++; continue;
			}

			if (current == tok_start) {
				addToken(tokens, TokenType::UNKNOWN, string(1, c), line, column);
				return NONE_OR_TRACEBACK({ line, column, "SyntaxError: Unexpected character" }, TRACEBACK_ERROR);
			}
		}

		size_t final_column = current - line_start + 1;
		while (indentStack.top() > 0) {
			indentStack.pop();
			addToken(tokens, TokenType::DEDENT, "", line, final_column);
		}
		if (!handle_LF) {
			addToken(tokens, TokenType::NEWLINE, "\\n", line, final_column);
		}
		addToken(tokens, TokenType::END_OF_FILE, "", line, final_column);

		return NONE_OR_TRACEBACK(0);
	}
}

using _pyrope::Token, _pyrope::TokenType, _pyrope::tokenize;

ostream& operator<<(ostream& os, TokenType type) {
	switch (type) {
	case TokenType::Type:
		os << "Type";
		break;
	case TokenType::Keyword:
		os << "Keyword";
		break;
	case TokenType::Identifier:
		os << "Identifier";
		break;
	case TokenType::LiteralString:
		os << "LiteralString";
		break;
	case TokenType::LiteralChar:
		os << "LiteralChar";
		break;
	case TokenType::LiteralNumber:
		os << "LiteralNumber";
		break;
	case TokenType::LiteralFloat:
		os << "LiteralFloat";
		break;
	case TokenType::LiteralBool:
		os << "LiteralBool";
		break;
	case TokenType::Operator:
		os << "Operator";
		break;
	case TokenType::Assignment:
		os << "Assignment";
		break;
	case TokenType::Punctuator:
		os << "Punctuator";
		break;
	case TokenType::Follow:
		os << "Follow";
		break;
	case TokenType::INDENT:
		os << "Indent";
		break;
	case TokenType::DEDENT:
		os << "Dedent";
		break;
	case TokenType::NEWLINE:
		os << "Newline";
		break;
	case TokenType::END_OF_FILE:
		os << "EOF";
		break;
	default:
		os << "Unknown";
	}
	return os;
}
ostream& operator<<(ostream& os, const Token& token) {
	os << token.line << ':' << token.column << '\t' << token.type << "\t\"" << token.lexeme << "\"";
	return os;
}
