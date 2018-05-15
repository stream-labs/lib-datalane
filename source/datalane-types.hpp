// Copyright(C) 2017-2018 Michael Fabian Dirks <info@xaymar.com>
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#pragma once
#include <inttypes.h>
#include <string>
#include <vector>
#include <map>

namespace datalane {
	enum class type : uint8_t {
		Void,
		Null = Void,
		Undefined = Void,
		// Base Types
		Bool,
		Int8,
		UInt8,
		Int16,
		UInt16,
		Int32,
		UInt32,
		Int64,
		UInt64,
		Float32,
		Float64,
		// Strings
		String8,
		String16,
		String32,
		// Binary
		Binary,
		// Other
		Array,
		Map,
	};

	struct value {
		datalane::type type;

		union {
			bool b;
			int8_t i8;
			uint8_t ui8;
			int16_t i16;
			uint16_t ui16;
			int32_t i32;
			uint32_t ui32;
			int64_t i64;
			uint64_t ui64;
			float_t f32;
			double_t f64;
		};
		// Strings
		std::string s8;
		std::u16string s16;
		std::u32string s32;
		// Binary
		std::vector<char> bin;
		// Other
		std::vector<value> arr;
		std::map<value, value> map;

		public:
		value() : ui64(0), type(type::Void) {}
		value(bool v) : b(v), type(type::Bool) {}
		value(int8_t v) : i8(v), type(type::Int8) {}
		value(uint8_t v) : ui8(v), type(type::UInt8) {}
		value(int16_t v) : i16(v), type(type::Int16) {}
		value(uint16_t v) : ui16(v), type(type::UInt16) {}
		value(int32_t v) : i32(v), type(type::Int32) {}
		value(uint32_t v) : ui32(v), type(type::UInt32) {}
		value(int64_t v) : i64(v), type(type::Int64) {}
		value(uint64_t v) : ui64(v), type(type::UInt64) {}
		value(float_t v) : f32(v), type(type::Float32) {}
		value(double_t v) : f64(v), type(type::Float64) {}
		value(std::string v) : ui64(0), s8(v), type(type::String8) {}
		value(char const* v) : ui64(0), s8(v ? v : ""), type(type::String8) {}
		value(std::u16string v) : ui64(0), s16(v), type(type::String16) {}
		value(char16_t const* v) : ui64(0), s16(v ? v : u""), type(type::String16) {}
		value(std::u32string v) : ui64(0), s32(v), type(type::String32) {}
		value(char32_t const* v) : ui64(0), s32(v ? v : U""), type(type::String32) {}
		value(std::vector<char> v) : ui64(0), bin(v), type(type::Binary) {}
		value(std::vector<value> v) : ui64(0), arr(v), type(type::Array) {}
		value(std::map<value, value> v) : ui64(0), map(v), type(type::Map) {}

		void clear();

		size_t size() const;
		size_t serialize(std::vector<char>& buffer, size_t const offset = 0) const;
		size_t deserialize(std::vector<char> const& buffer, size_t const offset = 0);
	};
}
