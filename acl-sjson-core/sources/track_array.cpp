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

#include "acl-sjson/track.h"
#include "acl-sjson/track_array.h"

namespace acl_sjson
{
	track_array::track_array()
    {
    }

    track_array::track_array(const char* name, const metadata_t& metadata)
        : m_name(name)
		, m_metadata(metadata)
    {
    }

	sample_type track_array::get_type() const
	{
		if (m_tracks.empty())
			return sample_type::unknown;

		return m_tracks[0].get_type();
	}

    size_t track_array::get_num_tracks() const
    {
        return m_tracks.size();
    }

	size_t track_array::get_num_samples_per_track() const
	{
		if (m_tracks.empty())
			return 0;

		return m_tracks[0].get_num_samples();
	}

	float track_array::get_sample_rate() const
	{
		if (m_tracks.empty())
			return 0.0F;

		return m_tracks[0].get_sample_rate();
	}

	float track_array::get_duration() const
	{
		if (m_tracks.empty())
			return 0.0F;

		return (m_tracks[0].get_num_samples() - 1) / m_tracks[0].get_sample_rate();
	}

    acl_version track_array::get_version() const
    {
        return m_metadata.version;
    }

	const metadata_t& track_array::get_metadata() const
	{
		return m_metadata;
	}

	const char* track_array::get_name() const
	{
		return m_name.c_str();
	}

    void track_array::emplace_back(track&& item)
    {
        m_tracks.emplace_back(std::move(item));
    }

    void track_array::clear()
    {
        m_tracks.clear();
    }

    track& track_array::operator[](size_t index)
    {
        return m_tracks[index];
    }

    const track& track_array::operator[](size_t index) const
    {
        return m_tracks[index];
    }
}
