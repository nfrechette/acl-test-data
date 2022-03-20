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
#include "acl-sjson/track.h"

namespace acl_sjson
{
    track::track(sample_type type, float sample_rate)
        : m_type(type)
        , m_sample_rate(sample_rate)
    {
    }

    sample_type track::get_type() const
    {
        return m_type;
    }

    size_t track::get_num_samples() const
    {
        return m_samples.size();
    }

    float track::get_sample_rate() const
    {
        return m_sample_rate;
    }

    void track::emplace_back(sample&& item)
    {
        m_samples.emplace_back(std::move(item));
    }

    track_description& track::get_description()
    {
        return m_desc;
    }

    const track_description& track::get_description() const
    {
        return m_desc;
    }

    sample& track::operator[](size_t index)
    {
        return m_samples[index];
    }

    const sample& track::operator[](size_t index) const
    {
        return m_samples[index];
    }
}
