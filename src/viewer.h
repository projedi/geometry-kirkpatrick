#pragma once

#include <boost/optional.hpp>

#include "visualization/viewer_adapter.h"

#include "kirkpatrick.h"

using geom::structures::point_type;

struct kirkpatrick_viewer : visualization::viewer_adapter {
   kirkpatrick_viewer(): _state(POLY_INPUT), _poly_complete(false), _query_hit(false) { }
   void draw(visualization::drawer_type& drawer) const;
   void print(visualization::printer_type& printer) const;
   bool on_double_click(point_type const& pt);
   bool on_key(int key);
private:
   void draw_poly_input(visualization::drawer_type& drawer) const;
   void draw_query(visualization::drawer_type& drawer) const;
private:
   enum viewer_state { POLY_INPUT, QUERY } _state;
   std::vector<point_type> _points;
   bool _poly_complete;
   // QUERY only
   boost::optional<kirkpatrick_type> _kirkpatrick;
   boost::optional<point_type> _query_point;
   bool _query_hit;
};
