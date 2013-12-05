#include <fstream>

#include <boost/none.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <QFileDialog>

#include "io/point.h"
#include "visualization/draw_util.h"

#include "viewer.h"

using namespace visualization;

kirkpatrick_viewer::kirkpatrick_viewer():
   _state(viewer_state::POLY_INPUT),
   _poly_complete(false),
   _query_hit(false) { }

void kirkpatrick_viewer::draw(drawer_type& drawer) const {
   size_t pt_size = 3;
   size_t line_size = 1;
   if(_kirkpatrick) _kirkpatrick->draw(drawer);
   if(!_points.empty()) {
      drawer.set_color(Qt::blue);
      auto fst = _points.begin();
      drawer.draw_point(*fst, pt_size);
      auto prev = fst;
      for(auto it = fst + 1; it != _points.end(); prev = it, ++it) {
         drawer.draw_point(*it, pt_size);
         drawer.draw_line(*prev, *it, line_size);
      }
      if(_poly_complete) drawer.draw_line(*prev, *fst, line_size);
   }
   if(_query_point) {
      drawer.set_color(Qt::red);
      drawer.draw_point(*_query_point, pt_size);
   }
}

void kirkpatrick_viewer::print(printer_type& printer) const {
   switch(_state) {
   case viewer_state::POLY_INPUT: printer.corner_stream() << "Polygon input state" << endl;
                                  break;
   case viewer_state::QUERY: printer.corner_stream() << "Query state" << endl;
                             break;
   }
   if(_query_point)
      printer.corner_stream() << endl << (_query_hit ? "" : "NOT ")
                              << "INSIDE" << endl;
   printer.corner_stream() << _status << endl;
}

bool check_point(point_type const& pt, std::vector<point_type> const& points) {
   segment_type s(pt, points.back());
   auto prev = points.begin();
   for(auto it = prev + 1; it != points.end(); prev = it, ++it) {
      if(*it == points.back()) continue;
      if(pt == *it) return false;
      if(intersects_inside(segment_type(*prev, *it), s)) return false;
   }
   return true;
}

bool kirkpatrick_viewer::on_double_click(point_type const& pt) {
   switch(_state) {
   case viewer_state::POLY_INPUT: add_point(pt); break;
   case viewer_state::QUERY: _query_point = pt;
                             _query_hit = _kirkpatrick->query(pt);
                             break;
   }
   return true;
}

bool kirkpatrick_viewer::on_key(int key) {
   switch(key) {
   case Qt::Key_Space: _state = viewer_state::POLY_INPUT;
                       _status = "";
                       _points.clear();
                       _poly_complete = false;
                       _kirkpatrick = boost::none;
                       _query_point = boost::none;
                       _query_hit = false;
                       return true;
   case Qt::Key_S: save(); return true;
   case Qt::Key_L: load(); return true;
   default: return false;
   }
}

size_t distance(point_type const& pt1, point_type const& pt2) {
   return (pt1 - pt2) * (pt1 - pt2);
}

void kirkpatrick_viewer::add_point(point_type const& point) {
   size_t dist = 15;
   if(_points.size() < 3) {
      for(auto pt: _points)
         if(distance(point, pt) < dist) {
            _status = "DO NOT CROSS LINES";
            return;
         }
      _points.push_back(point);
      _status = "";
   } else if(distance(_points.front(), point) < dist) {
      _poly_complete = true;
      _state = viewer_state::QUERY;
      _kirkpatrick = kirkpatrick_type(_points);
      _status = "";
   } else if(check_point(point, _points)) {
      _points.push_back(point);
      _status = "";
   } else _status = "DO NOT CROSS LINES";
}

void kirkpatrick_viewer::save() {
   std::string filename =
      QFileDialog::getSaveFileName(get_wnd(), "Save Points").toStdString();
   if(filename.empty()) return;
   std::ofstream ofs(filename.c_str());
   ofs << _poly_complete << std::endl;
   boost::copy(_points, std::ostream_iterator<point_type>(ofs, "\n"));
}

// TODO: I do not check correctness of loaded points.
void kirkpatrick_viewer::load() {
   std::string filename =
      QFileDialog::getOpenFileName(get_wnd(), "Load Points").toStdString();
   if(filename.empty()) return;
   std::ifstream ifs(filename.c_str());
   _points.clear();
   ifs >> _poly_complete;
   _points.assign(std::istream_iterator<point_type>(ifs),
         std::istream_iterator<point_type>());
   if(_poly_complete) {
      _state = viewer_state::QUERY;
      _query_point = boost::none;
      _kirkpatrick = kirkpatrick_type(_points);
   } else {
      _state = viewer_state::POLY_INPUT;
      _query_point = boost::none;
      _kirkpatrick = boost::none;
   }
   _status = "";
}
