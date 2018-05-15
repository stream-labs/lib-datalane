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

#include "datalane.hpp"
#include <sstream>

std::string datalane::convert_function_name_args_to_unique(std::string const& name, std::vector<type> const& param) {
	// Implement similar behavior to C/C++ compilers, which put parameter type
	//  into the generated function name in order to allow overloading of the
	//  same function, even when exported.
	// This behavior might not be desired, but allows some amount of flexibility.

	using namespace datalane;

	std::stringstream unique_name;
	unique_name << name;
	for (type p : param) {
		switch (p) {
			case type::Void:
				unique_name << "v";
				break;
			case type::Bool:
				unique_name << "b";
				break;
			case type::Int8:
				unique_name << "i8";
				break;
			case type::UInt8:
				unique_name << "u8";
				break;
			case type::Int16:
				unique_name << "i16";
				break;
			case type::UInt16:
				unique_name << "u16";
				break;
			case type::Int32:
				unique_name << "i32";
				break;
			case type::UInt32:
				unique_name << "u32";
				break;
			case type::Int64:
				unique_name << "i64";
				break;
			case type::UInt64:
				unique_name << "u64";
				break;
			case type::Float32:
				unique_name << "f32";
				break;
			case type::Float64:
				unique_name << "f64";
				break;
		}
	}

	return unique_name.str();
}
