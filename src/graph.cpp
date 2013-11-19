#include "graph.h"

#include "geom/primitives/point.h"
#include "geom/primitives/segment.h"

void graph_type::set_special_points(point_arr const& arr) {
   _special_points = arr;
}

void graph_type::add(point_type const& p) {
   _graph[p] = std::set<point_type>();
}

void graph_type::add(point_type const& p1, point_type const& p2) {
   std::cerr << "Adding " << p1 << " " << p2 << std::endl;
   if(_graph.find(p1) == _graph.end())
      throw std::logic_error("first point is not in graph");
   if(_graph.find(p2) == _graph.end())
      throw std::logic_error("second point is not in graph");
   _graph[p1].insert(p2);
   _graph[p2].insert(p1);
}

void graph_type::independent_set(size_t max_degree, point_arr& res) {
   std::set<point_type> masked;
   for(auto pt: _special_points) masked.insert(pt);
   for(auto el: _graph) {
      if(el.second.size() > max_degree) continue;
      if(masked.find(el.first) != masked.end()) continue;
      masked.insert(el.first);
      res.push_back(el.first);
   }
}

void graph_type::neighbours(point_type const& p, point_arr& res) {
   res.insert(res.begin(), _graph[p].begin(), _graph[p].end());
}

void graph_type::remove(point_arr const& pts) {
   for(auto pt: pts) {
      auto els = _graph[pt];
      for(auto el: els) {
         _graph[el].erase(pt);
      }
      _graph.erase(pt);
   }
}

std::vector<segment_type> graph_type::edges() const {
   std::vector<segment_type> res;
   for(auto el: _graph) {
      auto p1 = el.first;
      for(auto p2: el.second) {
         if(p2 < p1) continue;
         res.push_back(segment_type(p1, p2));
      }
   }
   return res;
}

void graph_type::dump() const {
   for(auto el: _graph) {
      std::cerr << el.first << ":";
      for(auto p2: el.second) {
         std::cerr << " " << p2;
      }
      std::cerr << std::endl;
   }
}
