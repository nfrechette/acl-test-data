#pragma once

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

#include "acl-sjson/sample.h"
#include "acl-sjson/track_description.h"

#include <cstddef>
#include <string>
#include <vector>

namespace acl_sjson
{
	class track
	{
	public:
		track(sample_type type, float sample_rate, const char* name);

		track(const track&) = delete;
		track(track&&) = default;
		track& operator=(const track&) = delete;
		track& operator=(track&&) = default;

		sample_type get_type() const;
		size_t get_num_samples() const;
		float get_sample_rate() const;
		const char* get_name() const;

		void emplace_back(sample&& item);

		track_description& get_description();
		const track_description& get_description() const;

		sample& operator[](size_t index);
		const sample& operator[](size_t index) const;

	private:
		std::vector<sample> m_samples;
		std::string			m_name;
		track_description   m_desc;
		sample_type         m_type;
		float               m_sample_rate;
	};
}
