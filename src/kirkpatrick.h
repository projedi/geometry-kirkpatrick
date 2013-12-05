#pragma once

#include "graph.h"
#include "util.h"

namespace visualization {
    struct drawer_type;
}

using visualization::drawer_type;

struct triangle_type;

struct kirkpatrick_type {
   kirkpatrick_type(point_arr const&);
   bool query(point_type const&) const;
   void draw(drawer_type& drawer) const;
private:
   point_arr _outer_points;
   graph_type _graph;
   std::shared_ptr<triangle_type> _top_triangle;
   std::vector<segment_type> _triangulation;
};
