#pragma once

#include <vector>

class kirkpatrick_impl;

struct kirkpatrick_type {
   kirkpatrick_type(std::vector<geom::structures::point_type> const&);
   bool query(geom::structures::point_type const&) const;
   void draw(visualization::drawer_type& drawer) const;
private:
   std::shared_ptr<kirkpatrick_impl> _impl;
};
