#ifndef INC_Display_H_
#define INC_Display_H_

#include <string>

class DataGet;

namespace Display {
  std::string dateForm(const DataGet& obs,
                       const std::string& id = "frm");

  int mainPage();
  int levels();
  int table();
  int plot();
  int info();
  int database();
} // Display

#endif // INC_Display_H_
