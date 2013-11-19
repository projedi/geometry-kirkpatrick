#include <memory>

#include "geom/primitives/point.h"
#include "geom/primitives/vector.h"
#include "visualization/viewer_adapter.h"

#include "graph.h"
#include "kirkpatrick.h"

using namespace geom::structures;

const size_t MAX_DEGREE = 8;

bool is_right_turn(point_type const& p1, point_type const& p2, point_type const& p3) {
   // 1   1   1
   // p1x p2x p3x
   // p1y p2y p3y
   int32_t det = (p2.x * p3.y - p3.x * p2.y) - (p1.x * p3.y - p3.x * p1.y) +
      (p1.x * p2.y - p2.x * p1.y);
   return det < 0;
}

bool inside_triangle(point_type const& p1, point_type const& p2, point_type const& p3,
      point_type const& pt) {
   return is_right_turn(pt, p2, p1) &&
          is_right_turn(pt, p3, p2) &&
          is_right_turn(pt, p1, p3);
}

struct triangle_type {
   triangle_type(point_type const& p1, point_type const& p2, point_type const& p3,
         bool is_base, bool is_inside): _p1(p1), _p2(p2), _p3(p3), _base(is_base),
         _inside(is_inside) { }
   bool query(point_type const& pt) const;
   point_type _p1;
   point_type _p2;
   point_type _p3;
   bool _base;
   bool _inside;
   std::vector<std::shared_ptr<triangle_type> > children;
   void dump() {
      std::cerr << _p1 << _p2 << _p3 << std::endl;
   }
};

std::ostream& operator<<(std::ostream& ost, triangle_type const& t) {
   return ost << "triangle(" << t._p1 << t._p2 << t._p3 << ")";
}

bool triangle_type::query(point_type const& pt) const {
   if(!inside_triangle(_p1, _p2, _p3, pt)) return false;
   if(_base) return _inside;
   for(auto t: children) {
      if(t->query(pt)) return true;
   }
   return false;
}

bool intersects_inside(segment_type const& s1, segment_type const& s2) {
   if(s1[0] == s2[0] || s1[0] == s2[1] || s1[1] == s2[0] || s1[1] == s2[1]) return false;
   bool r1 = is_right_turn(s1[0], s1[1], s2[0]);
   bool r2 = is_right_turn(s1[0], s1[1], s2[1]);
   bool r3 = is_right_turn(s2[0], s2[1], s1[0]);
   bool r4 = is_right_turn(s2[0], s2[1], s1[1]);
   return (r1 != r2) && (r3 != r4);
}

bool intersects(triangle_type const& t1, triangle_type const& t2) {
   segment_arr s1, s2;
   s1.push_back(segment_type(t1._p1, t1._p2));
   s1.push_back(segment_type(t1._p3, t1._p2));
   s1.push_back(segment_type(t1._p1, t1._p3));
   s2.push_back(segment_type(t2._p1, t2._p2));
   s2.push_back(segment_type(t2._p3, t2._p2));
   s2.push_back(segment_type(t2._p1, t2._p3));
   bool res = false;
   for(auto x: s1) {
      for(auto y: s2) {
         if(intersects_inside(x, y)) res = true;
      }
   }
   return res;
}

typedef std::set<std::shared_ptr<triangle_type> > triangle_set;
typedef std::map<point_type, triangle_set> triangle_map;

bool is_counter_clockwise(point_arr const& points) {
   size_t leftmost = 0;
   for(size_t i = 0; i != points.size(); ++i)
      if(points[i].x < points[leftmost].x) leftmost = i;
   size_t next = (leftmost + 1) % points.size();
   size_t prev = (points.size() + leftmost - 1) % points.size();
   return points[prev].y > points[next].y;
}

bool is_ear(point_type const& p1, point_type const& p2, point_type const& p3,
      point_arr const& points) {
   if(is_right_turn(p1, p2, p3)) return false;
   for(auto pt: points) {
      if(pt == p1 || pt == p2 || pt == p3) continue;
      if(inside_triangle(p1, p2, p3, pt)) return false;
   }
   return true;
}

void add_triangle(graph_type& graph, point_type const& p1, point_type const& p2,
      point_type const& p3, bool is_base, bool is_inside, triangle_map& triangles,
      triangle_set& generated_triangles) {
   graph.add(p1, p2);
   graph.add(p2, p3);
   graph.add(p3, p1);
   auto t = std::make_shared<triangle_type>(p1, p2, p3, is_base, is_inside);
   triangles[p1].insert(t);
   triangles[p2].insert(t);
   triangles[p3].insert(t);
   generated_triangles.insert(t);
}

// Ear clipping.
void triangulate_polygon(point_arr const& points, graph_type& graph,
      triangle_map& triangles, bool is_base, bool is_inside,
      triangle_set& generated_triangles) {
   point_arr avail_points;
   for(auto pt: points) {
      while(avail_points.size() > 1) {
         auto jt = avail_points.rbegin();
         if(!is_ear(*(jt + 1), *jt, pt, points)) break;
         std::cerr << *(jt + 1) << *jt << pt << " is a pocket" << std::endl;
         add_triangle(graph, *(jt + 1), *jt, pt, is_base, is_inside, triangles,
               generated_triangles);
         avail_points.pop_back();
      }
      std::cerr << "Adding " << pt << " to avail_points" << std::endl;
      avail_points.push_back(pt);
   }
}

void triangulate_pockets(point_arr const& points, graph_type& graph,
      point_arr& convex_hull, triangle_map& triangles) {
   triangle_set tmp;
   size_t leftmost = 0;
   for(size_t i = 0; i != points.size(); ++i) {
      if(points[i].x < points[leftmost].x) leftmost = i;
   }
   std::cerr << points[leftmost] << " is the leftmost" << std::endl;
   size_t i = leftmost;
   convex_hull.push_back(points[i++]);
   convex_hull.push_back(points[i++]);
   for(; i - leftmost != points.size() + 1; ++i) {
      auto pt = points[i % points.size()];
      while(convex_hull.size() > 1) {
         auto jt = convex_hull.rbegin();
         if(!is_right_turn(*(jt + 1), *jt, pt)) {
            convex_hull.push_back(pt);
            break;
         }
         add_triangle(graph, pt, *jt, *(jt + 1), true, false, triangles, tmp);
         convex_hull.pop_back();
      }
   }
}

bool is_visible(point_arr const& convex_hull, size_t i,
      point_arr const& outer_points, size_t j) {
   return is_right_turn(outer_points[j], convex_hull[i],
         convex_hull[(i + 1) % convex_hull.size()]);
}

// convex_hull and outer_points are counter-clockwise
void triangulate_with_outer_triangle(point_arr const& convex_hull,
      point_arr const& outer_points, graph_type& graph, triangle_map& triangles) {
   triangle_set tmp;
   // First point on convex_hull is leftmost.
   // Therefore it sees first and last out of outer_points.
   add_triangle(graph, convex_hull[0], outer_points[2], outer_points[0], true, false,
         triangles, tmp);
   size_t last_seen = 0;
   for(size_t i = 1; i != convex_hull.size(); ++i) {
      if(is_visible(convex_hull, i, outer_points, last_seen)) {
         add_triangle(graph, convex_hull[i - 1], outer_points[last_seen], convex_hull[i],
               true, false, triangles, tmp);
      }
      if(last_seen == 2) continue;
      if(is_visible(convex_hull, i, outer_points, last_seen + 1)) {
         add_triangle(graph, outer_points[last_seen], outer_points[last_seen + 1],
            convex_hull[i], true, false, triangles, tmp);
         last_seen += 1;
      }
   }
}

void initial_triangulation(point_arr const& points, point_arr const& outer_points,
      graph_type& graph, triangle_map& triangles) {
   std::cerr << "Triangulating polygon" << std::endl;
   triangle_set tris;
   triangulate_polygon(points, graph, triangles, true, true, tris);
   point_arr convex_hull;
   std::cerr << "Triangulating pockets" << std::endl;
   triangulate_pockets(points, graph, convex_hull, triangles);
   std::cerr << "Triangulating with outer triangle" << std::endl;
   triangulate_with_outer_triangle(convex_hull, outer_points, graph, triangles);
}

void sort(point_type const& pt, point_arr& poly) {
   std::sort(poly.begin(), poly.end(), [&pt](point_type const& p1, point_type const& p2) {
      double a1 = std::atan2(p1.y - pt.y, p1.x - pt.x);
      double a2 = std::atan2(p2.y - pt.y, p2.x - pt.x);
      return a1 < a2;
   });
}

void retriangulate(point_arr& poly, point_type const& pt,
      graph_type& graph, triangle_map& triangles) {
   triangle_set const& old_triangles = triangles[pt];
   triangle_set new_triangles;
   triangulate_polygon(poly, graph, triangles, false, false, new_triangles);
   for(auto ot: old_triangles) {
      for(auto nt: new_triangles) {
         if(intersects(*ot, *nt)) nt->children.push_back(ot);
      }
      for(auto el: triangles) {
         el.second.erase(ot);
      }
   }
}

std::shared_ptr<triangle_type> refinement(graph_type& graph, triangle_map& triangles,
      point_arr const& special_points) {
   for(;;) {
      point_arr iset;
      graph.independent_set(MAX_DEGREE, iset);
      std::cerr << "Found independent set of size " << iset.size() << std::endl;
      if(iset.empty()) break;
      for(auto pt: iset) {
         point_arr poly;
         graph.neighbours(pt, poly);
         sort(pt, poly);
         retriangulate(poly, pt, graph, triangles);
      }
      graph.remove(iset);
      std::cerr << "Removed independent set" << std::endl;
      graph.dump();
   }
   auto ts1 = triangles[special_points[0]];
   auto ts2 = triangles[special_points[1]];
   auto ts3 = triangles[special_points[2]];
   auto t = std::make_shared<triangle_type>(special_points[0], special_points[1],
         special_points[2], false, false);
   t->children.insert(t->children.end(), ts1.begin(), ts1.end());
   t->children.insert(t->children.end(), ts2.begin(), ts2.end());
   t->children.insert(t->children.end(), ts3.begin(), ts3.end());
   return t;
}

void build_outer_triangle(point_arr const& points, point_arr& outer_points) {
   point_type lower_left;
   int32_t c = 0;
   for(auto pt: points) {
      if(pt.x < lower_left.x) lower_left.x = pt.x;
      if(pt.y < lower_left.y) lower_left.y = pt.y;
      if(pt.x + pt.y > c) c = pt.x + pt.y;
   }
   lower_left += vector_type(-10,-10);
   c += 10;
   outer_points.push_back(lower_left);
   outer_points.push_back(point_type(c - lower_left.y, lower_left.y));
   outer_points.push_back(point_type(lower_left.x, c - lower_left.x));
}


struct kirkpatrick_impl {
   kirkpatrick_impl(point_arr const& points);
   bool query(point_type const& pt) const;
   void draw(visualization::drawer_type& drawer) const;
private:
   graph_type _graph;
   point_arr _outer_points;
   triangle_map _triangles;
   std::shared_ptr<triangle_type> _top_triangle;
};

kirkpatrick_impl::kirkpatrick_impl(point_arr const& points) {
   std::cerr << "Starting kirkpatrick" << std::endl;
   auto fst_point = points.begin();
   _graph.add(*fst_point);
   auto prev = fst_point;
   for(auto it = fst_point + 1; it != points.end(); ++it, ++prev) {
      _graph.add(*it);
      _graph.add(*prev, *it);
   }
   _graph.add(*prev, *fst_point);
   std::cerr << "Bootstrapped graph" << std::endl;

   build_outer_triangle(points, _outer_points);
   for(auto pt: _outer_points) _graph.add(pt);
   _graph.set_special_points(_outer_points);

   std::cerr << "Build outer triangle for graph" << std::endl;
   _graph.dump();

   if(is_counter_clockwise(points))
      initial_triangulation(points, _outer_points, _graph, _triangles);
   else {
      std::cerr << "Polygon was clockwise" << std::endl;
      point_arr res = points;
      std::reverse_copy(points.begin(), points.end(), res.begin());
      initial_triangulation(res, _outer_points, _graph, _triangles);
   }


   std::cerr << "Triangulated graph" << std::endl;
   _graph.dump();

   _top_triangle = refinement(_graph, _triangles, _outer_points);

   std::cerr << "Got top triangle" << std::endl;
}

bool kirkpatrick_impl::query(point_type const& pt) const {
   return _top_triangle->query(pt);
}

void kirkpatrick_impl::draw(visualization::drawer_type& drawer) const {
   //drawer.set_color(Qt::green);
   //drawer.draw_point(_outer_points[0], 3);
   //drawer.draw_point(_outer_points[1], 3);
   //drawer.draw_line(_outer_points[0], _outer_points[1], 1);
   //drawer.draw_point(_outer_points[2], 3);
   //drawer.draw_line(_outer_points[1], _outer_points[2], 1);
   //drawer.draw_line(_outer_points[2], _outer_points[0], 1);
   //drawer.set_color(Qt::gray);
   //for(auto segm: _graph.edges())
      //drawer.draw_line(segm[0], segm[1], 1);
}

kirkpatrick_type::kirkpatrick_type(point_arr const& points): _impl(new kirkpatrick_impl(points)) { }

bool kirkpatrick_type::query(point_type const& pt) const { return _impl->query(pt); }

void kirkpatrick_type::draw(visualization::drawer_type& drawer) const {
   return _impl->draw(drawer);
}
