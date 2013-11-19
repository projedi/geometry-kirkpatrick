#include <fstream>

#include <boost/none.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <QFileDialog>

#include "io/point.h"
#include "visualization/draw_util.h"

#include "viewer.h"

using namespace visualization;

void kirkpatrick_viewer::draw(drawer_type & drawer) const {
   if(!_points.empty()) {
      size_t pt_size = 3;
      size_t line_size = 1;
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
      drawer.draw_point(*_query_point, 3);
   }
   if(_kirkpatrick) {
      _kirkpatrick->draw(drawer);
   }
}

void kirkpatrick_viewer::print(printer_type & printer) const {
   switch(_state) {
   case POLY_INPUT: printer.corner_stream() << "Polygon input state" << endl;
                    break;
   case QUERY: printer.corner_stream() << "Query state" << endl;
               break;
   }
   if(_query_point) {
      printer.corner_stream() << endl << (_query_hit ? "" : "NOT ")
                              << "INSIDE" << endl;
   }
}

bool kirkpatrick_viewer::on_double_click(point_type const & pt) {
   switch(_state) {
   case POLY_INPUT: {
                    if(_points.size() < 3) {
                       _points.push_back(pt);
                       break;
                    }
                    auto fst = _points.front();
                    if((fst - pt) * (fst - pt) < 15) {
                       _poly_complete = true;
                       _state = QUERY;
                       _kirkpatrick = kirkpatrick_type(_points);
                       break;
                    } else {
                       _points.push_back(pt);
                       break;
                    }
                    }
   case QUERY: _query_point = pt;
               _query_hit = _kirkpatrick->query(pt);
               break;
   }
   return true;
}

bool kirkpatrick_viewer::on_key(int key) {
   switch(key) {
   case Qt::Key_Return: if(_state != QUERY) return true;
                        _query_point = boost::none;
                        _kirkpatrick = boost::none;
                        _query_hit = false;
                        _state = POLY_INPUT;
                        _points.clear();
                        _poly_complete = false;
                        return true;
   case Qt::Key_S: {
                   std::string filename =
                         QFileDialog::getSaveFileName(get_wnd(), "Save Points").toStdString();
                   if(filename.empty()) return true;
                   std::ofstream ofs(filename.c_str());
                   ofs << _poly_complete << std::endl;
                   boost::copy(_points, std::ostream_iterator<point_type>(ofs, "\n"));
                   return true;
                   }
   case Qt::Key_L: {
                   std::string filename =
                         QFileDialog::getOpenFileName(get_wnd(), "Load Points").toStdString();
                   if(filename.empty()) return true;
                   std::ifstream ifs(filename.c_str());
                   _points.clear();
                   ifs >> _poly_complete;
                   _points.assign(std::istream_iterator<point_type>(ifs),
                         std::istream_iterator<point_type>());
                   _state = QUERY;
                   _query_point = boost::none;
                   _kirkpatrick = boost::none;
                   return true;
                   }
   default: return false;
   }
}
