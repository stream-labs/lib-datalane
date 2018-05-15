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

#include "datalane-types.hpp"

void datalane::value::clear() {
	ui64 = 0;
	s8.clear();
	s16.clear();
	s32.clear();
	bin.clear();
	arr.clear();
	map.clear();
}

size_t datalane::value::size() const {
	size_t total_size = sizeof(datalane::type) + sizeof(uint64_t); // Type & Size Overhead
	switch (type) {
		case type::Void:
			break;
		case type::Bool:
			total_size += sizeof(bool);
			break;
		case type::Int8:
			total_size += sizeof(int8_t);
			break;
		case type::UInt8:
			total_size += sizeof(uint8_t);
			break;
		case type::Int16:
			total_size += sizeof(int16_t);
			break;
		case type::UInt16:
			total_size += sizeof(uint16_t);
			break;
		case type::Int32:
			total_size += sizeof(int32_t);
			break;
		case type::UInt32:
			total_size += sizeof(uint32_t);
			break;
		case type::Int64:
			total_size += sizeof(int64_t);
			break;
		case type::UInt64:
			total_size += sizeof(uint64_t);
			break;
		case type::Float32:
			total_size += sizeof(float_t);
			break;
		case type::Float64:
			total_size += sizeof(double_t);
			break;
		case type::String8:
			total_size += sizeof(size_t);
			total_size += s8.size();
			break;
		case type::String16:
			total_size += sizeof(size_t);
			total_size += s16.size();
			break;
		case type::String32:
			total_size += sizeof(size_t);
			total_size += s32.size();
			break;
		case type::Binary:
			total_size += sizeof(size_t);
			total_size += bin.size();
			break;
		case type::Array:
			total_size += sizeof(size_t);
			for (datalane::value const& v : arr) {
				total_size += v.size();
			}
			break;
		case type::Map:
			total_size += sizeof(size_t);
			for (std::pair<datalane::value, datalane::value> const& v : map) {
				total_size += v.first.size();
				total_size += v.second.size();
			}
			break;
	}
	return total_size;
}

size_t datalane::value::serialize(std::vector<char>& buffer, size_t const offset) const {
	size_t self_size = size();
	if ((buffer.size() - offset) < self_size) {
		return 0;
	}

	size_t position = offset;
	reinterpret_cast<uint64_t&>(buffer[position]) = self_size; position += sizeof(uint64_t);
	reinterpret_cast<datalane::type&>(buffer[position]) = type; position += sizeof(datalane::type);
	switch (type) {
		case type::Bool:
			reinterpret_cast<bool&>(buffer[position]) = b; position += sizeof(bool);
			break;
		case type::Int8:
			reinterpret_cast<int8_t&>(buffer[position]) = i8; position += sizeof(int8_t);
			break;
		case type::UInt8:
			reinterpret_cast<uint8_t&>(buffer[position]) = ui8; position += sizeof(uint8_t);
			break;
		case type::Int16:
			reinterpret_cast<int16_t&>(buffer[position]) = i16; position += sizeof(int16_t);
			break;
		case type::UInt16:
			reinterpret_cast<uint16_t&>(buffer[position]) = ui16; position += sizeof(uint16_t);
			break;
		case type::Int32:
			reinterpret_cast<int32_t&>(buffer[position]) = i32; position += sizeof(int32_t);
			break;
		case type::UInt32:
			reinterpret_cast<uint32_t&>(buffer[position]) = ui32; position += sizeof(uint32_t);
			break;
		case type::Int64:
			reinterpret_cast<int64_t&>(buffer[position]) = i64; position += sizeof(int64_t);
			break;
		case type::UInt64:
			reinterpret_cast<uint64_t&>(buffer[position]) = ui64; position += sizeof(uint64_t);
			break;
		case type::Float32:
			reinterpret_cast<float_t&>(buffer[position]) = f32; position += sizeof(float_t);
			break;
		case type::Float64:
			reinterpret_cast<double_t&>(buffer[position]) = f64; position += sizeof(double_t);
			break;
		case type::String8:
			reinterpret_cast<size_t&>(buffer[position]) = s8.size(); position += sizeof(size_t);
			if (s8.size() > 0) {
				memcpy(reinterpret_cast<char*>(buffer[position]),
					s8.data(),
					s8.size()); position += s8.size();
			}
			break;
		case type::String16:
			reinterpret_cast<size_t&>(buffer[position]) = s16.size(); position += sizeof(size_t);
			if (s16.size() > 0) {
				memcpy(reinterpret_cast<char*>(buffer[position]),
					s16.data(),
					s16.size()); position += s16.size();
			}
			break;
		case type::String32:
			reinterpret_cast<size_t&>(buffer[position]) = s32.size(); position += sizeof(size_t);
			if (s32.size() > 0) {
				memcpy(reinterpret_cast<char*>(buffer[position]),
					s32.data(),
					s32.size()); position += s32.size();
			}
			break;
		case type::Binary:
			reinterpret_cast<size_t&>(buffer[position]) = bin.size(); position += sizeof(size_t);
			if (bin.size() > 0) {
				memcpy(reinterpret_cast<char*>(buffer[position]),
					bin.data(),
					bin.size()); position += bin.size();
			}
			break;
		case type::Array:
			reinterpret_cast<size_t&>(buffer[position]) = arr.size(); position += sizeof(size_t);
			for (datalane::value const& v : arr) {
				size_t temp = v.serialize(buffer, position);
				if (temp == 0) {
					return 0;
				}
				position += temp;
			}
			break;
		case type::Map:
			reinterpret_cast<size_t&>(buffer[position]) = map.size(); position += sizeof(size_t);
			for (std::pair<datalane::value, datalane::value> const& v : map) {
				size_t temp = v.first.serialize(buffer, position);
				if (temp == 0) {
					return 0;
				}
				position += temp;
				temp = v.second.serialize(buffer, position);
				if (temp == 0) {
					return 0;
				}
				position += temp;
			}
			break;
	}

	return position - offset;
}

size_t datalane::value::deserialize(std::vector<char> const& buffer, size_t const offset) {
	if ((buffer.size() - offset) < sizeof(size_t)) {
		return 0;
	}

	clear();
	size_t position = offset;
	size_t self_size = reinterpret_cast<uint64_t const&>(buffer[position]); position += sizeof(uint64_t);

	if ((buffer.size() - offset) < self_size) {
		return 0;
	}

	this->type = reinterpret_cast<datalane::type const&>(buffer[position]); position += sizeof(datalane::type);
	switch (type) {
		case type::Bool:
			b = reinterpret_cast<bool const&>(buffer[position]); position += sizeof(bool);
			break;
		case type::Int8:
			i8 = reinterpret_cast<int8_t const&>(buffer[position]); position += sizeof(int8_t);
			break;
		case type::UInt8:
			ui8 = reinterpret_cast<uint8_t const&>(buffer[position]); position += sizeof(uint8_t);
			break;
		case type::Int16:
			i16 = reinterpret_cast<int16_t const&>(buffer[position]); position += sizeof(int16_t);
			break;
		case type::UInt16:
			ui16 = reinterpret_cast<uint16_t const&>(buffer[position]); position += sizeof(uint16_t);
			break;
		case type::Int32:
			i32 = reinterpret_cast<int32_t const&>(buffer[position]); position += sizeof(int32_t);
			break;
		case type::UInt32:
			ui32 = reinterpret_cast<uint32_t const&>(buffer[position]); position += sizeof(uint32_t);
			break;
		case type::Int64:
			i64 = reinterpret_cast<int64_t const&>(buffer[position]); position += sizeof(int64_t);
			break;
		case type::UInt64:
			ui64 = reinterpret_cast<uint64_t const&>(buffer[position]); position += sizeof(uint64_t);
			break;
		case type::Float32:
			f32 = reinterpret_cast<float_t const&>(buffer[position]); position += sizeof(float_t);
			break;
		case type::Float64:
			f64 = reinterpret_cast<double_t const&>(buffer[position]); position += sizeof(double_t);
			break;
		case type::String8:
		{
			size_t length = reinterpret_cast<size_t const&>(buffer[position]); position += sizeof(size_t);
			if (length > 0) {
				s8 = std::string(reinterpret_cast<char const*>(buffer[position]), length);
			}
			break;
		}
		case type::String16:
		{
			size_t length = reinterpret_cast<size_t const&>(buffer[position]); position += sizeof(size_t);
			if (length > 0) {
				s16 = std::u16string(reinterpret_cast<char16_t const*>(buffer[position]), length);
			}
			break;
		}
		case type::String32:
		{
			size_t length = reinterpret_cast<size_t const&>(buffer[position]); position += sizeof(size_t);
			if (length > 0) {
				s32 = std::u32string(reinterpret_cast<char32_t const*>(buffer[position]), length);
			}
			break;
		}
		case type::Binary:
		{
			size_t length = reinterpret_cast<size_t const&>(buffer[position]); position += sizeof(size_t);
			if (length > 0) {
				bin = std::vector<char>(reinterpret_cast<char const*>(buffer[position]), 
					reinterpret_cast<char const*>(buffer[position]) + length);
			}
			break;
		}
		case type::Array:
		{
			size_t length = reinterpret_cast<size_t const&>(buffer[position]); position += sizeof(size_t);
			for (size_t idx = 0; idx < length; idx++) {
				datalane::value v;
				size_t temp = v.deserialize(buffer, position);
				if (temp == 0) {
					return 0;
				}
				position += temp;
				arr.push_back(std::move(v));
			}
			break;
		}
		case type::Map:
		{
			size_t length = reinterpret_cast<size_t const&>(buffer[position]); position += sizeof(size_t);
			for (size_t idx = 0; idx < length; idx++) {
				datalane::value left, right;
				size_t temp = left.deserialize(buffer, position);
				if (temp == 0) {
					return 0;
				}
				position += temp;
				temp = right.deserialize(buffer, position);
				if (temp == 0) {
					return 0;
				}
				position += temp;
				map.insert(std::make_pair(std::move(left), std::move(right)));
			}
			break;
		}
	}

	return position - offset;
}
