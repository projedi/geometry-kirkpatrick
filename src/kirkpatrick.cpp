#include <algorithm> // for std::max in "geom/primitives/vector.h"
#include "geom/primitives/vector.h"
#include "visualization/viewer_adapter.h"

using geom::structures::vector_type;

#include "kirkpatrick.h"
#include "triangle.h"

const size_t MAX_DEGREE = 8;

typedef std::set<std::shared_ptr<triangle_type> > triangle_set;

std::ostream& operator<<(std::ostream& ost, triangle_set const& triangles) {
   for(auto i: triangles) {
      ost << "   " << *i << std::endl;
   }
   return ost;
}

typedef std::map<point_type, triangle_set> triangle_map;

std::ostream& operator<<(std::ostream& ost, triangle_map const& triangles) {
   for(auto i: triangles) {
      ost << i.first << ":" << std::endl << i.second;
   }
   return ost;
}

void add_triangle(graph_type& graph, point_type const& p1, point_type const& p2,
      point_type const& p3, bool is_inside, triangle_map& triangles,
      triangle_set& generated_triangles) {
   logger << "Adding triangle " << p1 << " " << p2 << " " << p3 << std::endl;
   graph.add_edge(p1, p2);
   graph.add_edge(p2, p3);
   graph.add_edge(p3, p1);
   auto t = std::make_shared<triangle_type>(p1, p2, p3, is_inside);
   triangles[p1].insert(t);
   triangles[p2].insert(t);
   triangles[p3].insert(t);
   generated_triangles.insert(t);
}

// Ear clipping.
void triangulate_polygon(point_arr const& points, graph_type& graph,
      triangle_map& triangles, bool is_inside,
      triangle_set& generated_triangles) {
   point_arr avail_points;
   for(auto pt: points) {
      while(avail_points.size() > 1) {
         auto jt = avail_points.rbegin();
         if(!is_ear(*(jt + 1), *jt, pt, points)) break;
         logger << *(jt + 1) << *jt << pt << " is an ear" << std::endl;
         add_triangle(graph, *(jt + 1), *jt, pt, is_inside, triangles,
               generated_triangles);
         avail_points.pop_back();
      }
      logger << "Adding " << pt << " to avail_points" << std::endl;
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
   logger << points[leftmost] << " is the leftmost" << std::endl;
   size_t i = leftmost;
   convex_hull.push_back(points[(i++) % points.size()]);
   logger << "Pushing " << convex_hull.back() << " to convex_hull" << std::endl;
   convex_hull.push_back(points[(i++) % points.size()]);
   logger << "Pushing " << convex_hull.back() << " to convex_hull" << std::endl;
   for(; i - leftmost != points.size() + 1; ++i) {
      auto pt = points[i % points.size()];
      while(convex_hull.size() > 1) {
         auto jt = convex_hull.rbegin();
         if(!is_right_turn(*(jt + 1), *jt, pt)) break;
         bool res = true;
         for(auto p: points) {
            if(p == *(jt + 1) || p == *jt || p == pt) continue;
            if(inside_triangle(pt, *jt, *(jt + 1), p)) { res = false; break; }
         }
         if(!res) break;
         logger << pt << *jt << *(jt + 1) << " is a pocket" << std::endl;
         add_triangle(graph, pt, *jt, *(jt + 1), false, triangles, tmp);
         logger << "Popping " << convex_hull.back() << " from convex_hull" << std::endl;
         convex_hull.pop_back();
      }
      convex_hull.push_back(pt);
      logger << "Pushing " << pt << " to convex_hull" << std::endl;
   }
}

// convex_hull and outer_points are counter-clockwise
void triangulate_with_outer_triangle(point_arr const& convex_hull,
      point_arr const& outer_points, graph_type& graph, triangle_map& triangles) {
   triangle_set tmp;
   // First point on convex_hull is leftmost.
   // Therefore it sees first and last out of outer_points.
   add_triangle(graph, convex_hull[0], outer_points[2], outer_points[0], false,
         triangles, tmp);
   size_t last_seen = 0;
   for(size_t i = 1; i != convex_hull.size(); ++i) {
      logger << "Looking at " << convex_hull[i] << std::endl;
      if(is_left_turn(outer_points[last_seen], convex_hull[i], convex_hull[i - 1])) {
         logger << "It sees " << last_seen << std::endl;
         add_triangle(graph, convex_hull[i - 1], outer_points[last_seen], convex_hull[i],
               false, triangles, tmp);
      }
      if(last_seen == 2) continue;
      if(is_right_turn(outer_points[last_seen + 1], convex_hull[i], convex_hull[i + 1])) {
         logger << "And it sees " << last_seen + 1 << std::endl;
         add_triangle(graph, outer_points[last_seen], outer_points[last_seen + 1],
            convex_hull[i], false, triangles, tmp);
         last_seen += 1;
      }
   }
}

void initial_triangulation(point_arr const& points, point_arr const& outer_points,
      graph_type& graph, triangle_map& triangles) {
   logger << "Triangulating polygon" << std::endl;
   triangle_set tris;
   triangulate_polygon(points, graph, triangles, true, tris);
   point_arr convex_hull;
   logger << "Triangulating pockets" << std::endl;
   triangulate_pockets(points, graph, convex_hull, triangles);
   logger << "Triangulating with outer triangle" << std::endl;
   triangulate_with_outer_triangle(convex_hull, outer_points, graph, triangles);
}


void retriangulate(point_arr& poly, point_type const& pt,
      graph_type& graph, triangle_map& triangles) {
   triangle_set const& old_triangles = triangles[pt];
   logger << "Retriangulation for " << pt << ". Old set: " << std::endl;
   logger << old_triangles << std::endl;
   triangle_set new_triangles;
   triangulate_polygon(poly, graph, triangles, false, new_triangles);
   logger << "New set " << std::endl << new_triangles << std::endl;
   for(auto ot: old_triangles) {
      for(auto nt: new_triangles) {
         if(intersects(*ot, *nt)) {
            nt->add_child(ot);
            logger << "Intersection between " << *ot << " and " << *nt << std::endl;
         }
      }
      for(auto& el: triangles) {
         logger << "Erasing " << *ot << " from " << el.first << std::endl;
         size_t res = el.second.erase(ot);
         logger << "Erased " << res << std::endl;
      }
   }
   triangles.erase(pt);
}


bool refine(graph_type& graph, triangle_map& triangles) {
   point_arr iset = graph.independent_set(MAX_DEGREE);
   if(iset.empty()) return false;
   logger << "Found independent set of size " << iset.size() << std::endl;
   for(auto pt: iset) {
      logger << "Working on " << pt << std::endl;
      point_arr poly = sort_counter_clockwise(pt, graph.neighbours(pt));
      logger << "Neighbours: ";
      for(auto p: poly) {
         logger << p << " ";
      }
      logger << std::endl;
      retriangulate(poly, pt, graph, triangles);
   }
   graph.remove(iset);
   logger << "Removed independent set" << std::endl;
   return true;
}

std::shared_ptr<triangle_type> refinement(graph_type& graph, triangle_map& triangles,
      point_arr const& special_points) {
   for(;;) {
      if(!refine(graph, triangles)) break;
   }
   return *(triangles.begin()->second.begin());
}

point_arr find_outer_triangle(point_arr const& points) {
   point_arr res;
   point_type lower_left;
   int32_t c = 0;
   for(auto pt: points) {
      if(pt.x < lower_left.x) lower_left.x = pt.x;
      if(pt.y < lower_left.y) lower_left.y = pt.y;
      if(pt.x + pt.y > c) c = pt.x + pt.y;
   }
   lower_left += vector_type(-10,-10);
   c += 10;
   res.push_back(lower_left);
   res.push_back(point_type(c - lower_left.y, lower_left.y));
   res.push_back(point_type(lower_left.x, c - lower_left.x));
   return res;
}

kirkpatrick_type::kirkpatrick_type(point_arr const& points):
   _outer_points(find_outer_triangle(points)),
   _graph(_outer_points) {
   logger << "Starting kirkpatrick" << std::endl;
   _graph.add_poly(points);
   logger << "Bootstrapped graph: " << std::endl << _graph << std::endl;

   point_arr points_copy = points;
   if(!is_counter_clockwise(points)) {
      logger << "Polygon was clockwise" << std::endl;
      std::reverse_copy(points.begin(), points.end(), points_copy.begin());
   }

   triangle_map triangles;
   initial_triangulation(points_copy, _outer_points, _graph, triangles);
   logger << "Triangulated graph: " << std::endl << _graph << std::endl;
   _triangulation = _graph.edges();
   logger << triangles << std::endl;
   _top_triangle = refinement(_graph, triangles, _outer_points);

   logger << "Got top triangle" << std::endl;
}

bool kirkpatrick_type::query(point_type const& pt) const {
   return _top_triangle->query(pt);
}

void kirkpatrick_type::draw(visualization::drawer_type& drawer) const {
   drawer.set_color(Qt::gray);
   for(auto segm: _triangulation) {
      drawer.draw_line(segm[0], segm[1], 1);
   }
}
