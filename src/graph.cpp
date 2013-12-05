#include "graph.h"

graph_type::graph_type(point_arr const& special_points):
   _special_points(special_points) {
   add_poly(special_points);
}

void graph_type::add(point_type const& p) {
   logger << "Adding point " << p << std::endl;
   _graph[p] = std::set<point_type>();
}

void graph_type::add_edge(point_type const& p1, point_type const& p2) {
   logger << "Adding edge " << p1 << " <-> " << p2 << std::endl;
   if(_graph.find(p1) == _graph.end())
      throw std::logic_error("first point is not in graph");
   if(_graph.find(p2) == _graph.end())
      throw std::logic_error("second point is not in graph");
   _graph[p1].insert(p2);
   _graph[p2].insert(p1);
}

void graph_type::add_poly(point_arr const& points) {
   auto fst_point = points.begin();
   add(*fst_point);
   auto prev = fst_point;
   for(auto it = fst_point + 1; it != points.end(); ++it, ++prev) {
      add(*it);
      add_edge(*prev, *it);
   }
   add_edge(*prev, *fst_point);
}

segment_arr graph_type::edges() const {
   segment_arr res;
   for(auto el: _graph) {
      auto p1 = el.first;
      for(auto p2: el.second) {
         if(p2 < p1) continue;
         res.push_back(segment_type(p1, p2));
      }
   }
   return res;
}

point_arr graph_type::independent_set(size_t max_degree) const {
   point_arr res;
   std::set<point_type> masked;
   for(auto pt: _special_points) masked.insert(pt);
   for(auto el: _graph) {
      if(el.second.size() > max_degree) continue;
      if(masked.find(el.first) != masked.end()) continue;
      masked.insert(el.second.begin(), el.second.end());
      res.push_back(el.first);
   }
   return res;
}

void graph_type::remove(point_type const& pt) {
   for(auto el: _graph[pt]) {
      _graph[el].erase(pt);
   }
   _graph.erase(pt);
}

void graph_type::remove(point_arr const& pts) {
   for(auto pt: pts) remove(pt);
}
