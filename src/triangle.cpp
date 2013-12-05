#include "triangle.h"

bool intersects(triangle_type const& t1, triangle_type const& t2) {
   segment_arr s1, s2;
   s1.push_back(segment_type(t1.p1(), t1.p2()));
   s1.push_back(segment_type(t1.p3(), t1.p2()));
   s1.push_back(segment_type(t1.p1(), t1.p3()));
   s2.push_back(segment_type(t2.p1(), t2.p2()));
   s2.push_back(segment_type(t2.p3(), t2.p2()));
   s2.push_back(segment_type(t2.p1(), t2.p3()));
   bool res = false;
   for(auto x: s1) {
      for(auto y: s2) {
         if(intersects(x, y)) res = true;
      }
   }
   return res;
}

bool triangle_type::inside(point_type const& pt) const {
   return inside_triangle(_p1, _p2, _p3, pt);
}

bool triangle_type::query(point_type const& pt) const {
   logger << "Querying " << pt << " inside " << *this << std::endl;
   if(!inside(pt)) return false;
   logger << "Point is inside" << std::endl;
   if(_children.empty()) return _is_inside;
   logger << "Iterating over children" << std::endl;
   for(auto t: _children) {
      if(t->query(pt)) return true;
   }
   return false;
}
