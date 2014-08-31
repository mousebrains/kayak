#ifndef INC_MyException_H_
#define INC_MyException_H_

#include <exception>


struct MyException : public std::exception {
  std::string mReason;
  MyException(const std::string& str) : mReason(str) {}
  ~MyException() throw () {}
  const char *what() const throw() {return mReason.c_str();}
};

#endif // INC_MyException_H_
