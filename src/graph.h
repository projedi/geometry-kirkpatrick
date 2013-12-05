#pragma once

#include <map>
#include <set>

#include <boost/optional.hpp>

#include "util.h"

struct graph_type {
   graph_type(point_arr const& special_points);
   void add(point_type const&);
   void add_edge(segment_type const& e) { add_edge(e[0], e[1]); }
   void add_edge(point_type const&, point_type const&);
   void add_poly(point_arr const&);
   segment_arr edges() const;
   point_arr independent_set(size_t max_degree) const;
   std::set<point_type> neighbours(point_type const& p) const { return _graph.at(p); }
   void remove(point_type const&);
   void remove(point_arr const&);
   friend std::ostream& operator<<(std::ostream&, graph_type const&);
private:
   std::map<point_type, std::set<point_type> > _graph;
   point_arr _special_points;
};

inline std::ostream& operator<<(std::ostream& ost, graph_type const& graph) {
   for(auto el: graph._graph) {
      ost << el.first << ":";
      for(auto p2: el.second) {
         ost << " " << p2;
      }
      ost << std::endl;
   }
   return ost;
}
