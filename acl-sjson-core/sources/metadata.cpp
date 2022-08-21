////////////////////////////////////////////////////////////////////////////////
// The MIT License (MIT)
//
// Copyright (c) 2022 Nicholas Frechette
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include "acl-sjson/metadata.h"

namespace acl_sjson
{
	const char* to_string(rotation_format_t format)
	{
		switch (format)
		{
		case rotation_format_t::quatf_full:				return "quatf full";
		case rotation_format_t::quatf_drop_w_full:		return "quatf full no W";
		case rotation_format_t::quatf_drop_w_variable:	return "quatf variable no W";
		default:										return "unknown";
		}
	}

	const char* to_string(vector_format_t format)
	{
		switch (format)
		{
		case vector_format_t::vector3f_full:		return "vector3f full";
		case vector_format_t::vector3f_variable:	return "vector3f variable";
		default:									return "unknown";
		}
	}

	const char* to_string(track_variant_t variant)
	{
		switch (variant)
		{
		case track_variant_t::transform:	return "transform";
		case track_variant_t::scalar:		return "scalar";
		default:							return "unknown";
		}
	}
}
