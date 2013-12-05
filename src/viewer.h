#pragma once

#include <boost/optional.hpp>

#include "visualization/viewer_adapter.h"

#include "kirkpatrick.h"

using geom::structures::point_type;

struct kirkpatrick_viewer : visualization::viewer_adapter {
   kirkpatrick_viewer();
   void draw(visualization::drawer_type&) const;
   void print(visualization::printer_type&) const;
   bool on_double_click(point_type const&);
   bool on_key(int key);
private:
   void add_point(point_type const&);
   void save();
   void load();
private:
   enum class viewer_state { POLY_INPUT, QUERY } _state;
   std::string _status;
   std::vector<point_type> _points;
   bool _poly_complete;
   // QUERY only
   boost::optional<kirkpatrick_type> _kirkpatrick;
   boost::optional<point_type> _query_point;
   bool _query_hit;
};
