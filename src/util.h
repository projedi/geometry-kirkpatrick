#pragma once

#include <iostream>
#include <fstream>

#include "geom/primitives/point.h"
#include "geom/primitives/segment.h"
#include "io/point.h"
#include "io/segment.h"

using geom::structures::point_type;
using geom::structures::segment_type;

#ifdef DEBUG
#define logger std::cerr
#else
struct null_stream: std::ostream {
   null_stream(): std::ostream(new std::filebuf()) { }
};

static null_stream logger;

// Ensure this operation doesn't cost us (almost) anything.
template<class T>
null_stream& operator<<(null_stream& ost, T const&) { return ost; }
#endif

typedef std::vector<point_type> point_arr;
typedef std::vector<segment_type> segment_arr;

// 1   1   1
// p1x p2x p3x
// p1y p2y p3y
inline float determinant(point_type const& p1, point_type const& p2, point_type const& p3) {
   return (p2.x * p3.y - p3.x * p2.y) - (p1.x * p3.y - p3.x * p1.y) +
      (p1.x * p2.y - p2.x * p1.y);
}

template<class T>
int sign(T t) {
   if(t < 0) return -1;
   if(t == 0) return 0;
   else return 1;
}

inline bool is_right_turn(point_type const& p1, point_type const& p2,
      point_type const& p3) {
   return determinant(p1, p2, p3) < 0;
}

inline bool is_left_turn(point_type const& p1, point_type const& p2,
      point_type const& p3) {
   return determinant(p1, p2, p3) > 0;
}

inline bool intersects(segment_type const& s1, segment_type const& s2) {
   int r1 = sign(determinant(s1[0], s1[1], s2[0]));
   int r2 = sign(determinant(s1[0], s1[1], s2[1]));
   int r3 = sign(determinant(s2[0], s2[1], s1[0]));
   int r4 = sign(determinant(s2[0], s2[1], s1[1]));
   return (r1 * r2 <= 0) && (r3 * r4 <= 0);
}

inline bool intersects_inside(segment_type const& s1, segment_type const& s2) {
   if(s1[0] == s2[0] || s1[0] == s2[1] || s1[1] == s2[0] || s1[1] == s2[1]) return false;
   return intersects(s1, s2);
}

inline bool is_counter_clockwise(point_arr const& points) {
   size_t leftmost = 0;
   for(size_t i = 0; i != points.size(); ++i)
      if(points[i].x < points[leftmost].x) leftmost = i;
   size_t next = (leftmost + 1) % points.size();
   size_t prev = (points.size() + leftmost - 1) % points.size();
   return points[prev].y > points[next].y;
}

inline bool is_visible(point_arr const& convex_hull, size_t i,
      point_arr const& outer_points, size_t j) {
   return is_right_turn(outer_points[j], convex_hull[i],
         convex_hull[(i + 1) % convex_hull.size()]);
}

template<class Cont>
point_arr sort_counter_clockwise(point_type const& pt, Cont const& points) {
   point_arr res(points.size());
   std::partial_sort_copy(points.begin(), points.end(), res.begin(), res.end(),
         [&pt](point_type const& p1, point_type const& p2) {
            double a1 = std::atan2(p1.y - pt.y, p1.x - pt.x);
            double a2 = std::atan2(p2.y - pt.y, p2.x - pt.x);
            return a1 < a2;
   });
   return res;
}

inline bool inside_triangle(point_type const& p1, point_type const& p2,
      point_type const& p3, point_type const& pt) {
   int r1 = sign(determinant(pt, p2, p1));
   int r2 = sign(determinant(pt, p3, p2));
   int r3 = sign(determinant(pt, p1, p3));
   return (r1 <= 0 && r2 <= 0 && r3 <= 0);
}

inline bool is_ear(point_type const& p1, point_type const& p2, point_type const& p3,
      point_arr const& points) {
   if(!is_left_turn(p1, p2, p3)) return false;
   for(auto pt: points) {
      if(pt == p1 || pt == p2 || pt == p3) continue;
      if(inside_triangle(p1, p2, p3, pt)) return false;
   }
   return true;
}
