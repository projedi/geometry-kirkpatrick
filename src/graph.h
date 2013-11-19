#pragma once

#include <map>
#include <set>

#include <boost/optional.hpp>

#include "geom/primitives/point.h"

namespace visualization {
    class drawer_type;
}

namespace geom {
    namespace structures {
        class point_type;
        class segment_type;
    }
}

using geom::structures::point_type;
using geom::structures::segment_type;
using visualization::drawer_type;

typedef std::vector<point_type> point_arr;
typedef std::vector<segment_type> segment_arr;

struct graph_type {
   void set_special_points(point_arr const& arr);
   void add(point_type const& p);
   void add(point_type const& p1, point_type const& p2);
   void independent_set(size_t max_degree, point_arr&);
   void neighbours(point_type const&, point_arr&);
   void remove(point_arr const&);
   std::vector<segment_type> edges() const;
   void dump() const;
private:
   std::map<point_type, std::set<point_type> > _graph;
   point_arr _special_points;
};

inline std::ostream& operator<<(std::ostream& ost, point_type const& p) {
   return ost << "(" << p.x << ", " << p.y << ")";
}
