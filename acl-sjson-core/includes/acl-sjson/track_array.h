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

#include "acl-sjson/acl_version.h"
#include "acl-sjson/track.h"

#include <cstddef>
#include <vector>

namespace acl_sjson
{
    class track_array
    {
    public:
        track_array(acl_version version);

        track_array(const track_array&) = delete;
        track_array(track_array&&) = default;
        track_array& operator=(const track_array&) = delete;
        track_array& operator=(track_array&&) = default;

        size_t get_num_tracks() const;
		size_t get_num_samples_per_track() const;
		float get_sample_rate() const;
        acl_version get_version() const;

        void emplace_back(track&& item);

        void clear();

        track& operator[](size_t index);
        const track& operator[](size_t index) const;

    private:
        std::vector<track>  m_tracks;
        acl_version         m_version;
    };
}
