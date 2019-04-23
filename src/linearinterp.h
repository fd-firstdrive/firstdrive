/************************************************************************/
/*                                                                      */
/* This file is part of FirstDrive.                                         */
/*                                                                      */
/* FirstDrive is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* FirstDrive is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with FirstDrive.  If not, see <http://www.gnu.org/licenses/>.      */
/*                                                                      */
/************************************************************************/

#ifndef _LINEARINTERP_H
#define _LINEARINTERP_H

#include "pairsort.h"

#include <vector>
#include <map>
#include <algorithm>
#include <cassert>

template <typename T>
class LinearInterp
{
public:
	enum BoundaryEnum
	{
		CONSTANTSLOPE,
  		CONSTANTVALUE
	};

private:
	std::vector <std::pair <T, T> > points;
	mutable T first_slope;
	mutable T last_slope;
	mutable bool slopes_calculated;
	BoundaryEnum mode;
	T empty_value;

	void Calculate() const
	{
		size_t n = points.size ();
		assert (n > 1);

		first_slope = (points[1].second-points[0].second)/(points[1].first-points[0].first);
		last_slope = (points[n-1].second-points[n-2].second)/(points[n-1].first-points[n-2].first);

		slopes_calculated = true;
	}

public:
	LinearInterp() : first_slope(0.0), last_slope(0.0), slopes_calculated(false), mode(CONSTANTVALUE), empty_value(0) {}
	LinearInterp(T empty_value_) : first_slope(0.0), last_slope(0.0), slopes_calculated(false), mode(CONSTANTVALUE), empty_value(empty_value_) {} ///< Interpolate will return the given empty_value if no points exist

	void Clear()
	{
		points.clear();
		slopes_calculated = false;
	}

	void AddPoint(const T x, const T y)
	{
		points.push_back(std::pair <T,T> (x,y));
		slopes_calculated = false;
		PairSortFirst <T> sorter;
		std::sort(points.begin(), points.end(), sorter);
	}

	T Interpolate(T x) const
	{
		if (points.empty())
			return empty_value;

		if (points.size() == 1)
		{
			return points[0].second;
		}

		// calculate() only needs to be called once for a given set of
		// points.
		if ( !slopes_calculated )
			Calculate ();

		size_t low = 0;
		size_t high = points.size () - 1;
		size_t index;

		// handle the case where the value is out of bounds
		if (x > points[high].first)
		{
			if (mode == CONSTANTSLOPE)
				return points[high].second+last_slope*(x-points[high].first);
			else
				return points[high].second;
		}
		if (x < points[low].first)
		{
			if (mode == CONSTANTSLOPE)
				return points[low].second+first_slope*(x-points[low].first);
			else
				return points[low].second;
		}

		// Bisect to find the interval that distance is on.
		while ( ( high - low ) > 1 )
		{
			index = size_t ( ( high + low ) / 2.0 );
			if ( points [index].first > x )
				high = index;
			else
				low = index;
		}

		// Make sure that x_high > x_low.
		const T diff = points [high].first - points [low].first;
		assert ( diff >= 0.0 );
		const T diffy = points [high].second - points [low].second;

		return diffy*(x-points[low].first)/diff+points[low].second;
	}

	/// if the mode is set to CONSTANTSLOPE, then values outside of the bounds will be extrapolated based
	/// on the slope of the closest points.  if set to CONSTANTVALUE, the values outside of the bounds will
	/// be set to the value of the closest point.
	void SetBoundaryMode ( const BoundaryEnum& value )
	{
		mode = value;
	}

};

#endif
