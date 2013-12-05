#pragma once

#include "util.h"

struct triangle_type;
typedef std::shared_ptr<triangle_type> triangle_ptr;

struct triangle_type {
   triangle_type(point_type const& p1, point_type const& p2, point_type const& p3,
         bool is_inside): _p1(p1), _p2(p2), _p3(p3), _is_inside(is_inside) { }
   bool inside(point_type const& pt) const;
   bool query(point_type const& pt) const;
   void add_child(triangle_ptr const& t) { _children.push_back(t); }
   template<class Cont>
   void add_children(Cont const& ts);
   point_type const& p1() const { return _p1; }
   point_type const& p2() const { return _p2; }
   point_type const& p3() const { return _p3; }
   friend std::ostream& operator<<(std::ostream&, triangle_type const&);
private:
   point_type _p1;
   point_type _p2;
   point_type _p3;
   std::vector<triangle_ptr> _children;
   bool _is_inside;
};

bool intersects(triangle_type const& t1, triangle_type const& t2);

inline std::ostream& operator<<(std::ostream& ost, triangle_type const& t) {
   ost << "triangle { " << t._p1 << " " << t._p2 << " " << t._p3 << " }: " << std::endl;
   for(auto tr: t._children) {
      ost << "      triangle { " << tr->_p1 << " " << tr->_p2 << " " << tr->_p3 << " }"
          << std::endl;
   }
   return ost;
}

template<class Cont>
void triangle_type::add_children(Cont const& ts) {
   _children.insert(_children.end(), ts.begin(), ts.end());
}
