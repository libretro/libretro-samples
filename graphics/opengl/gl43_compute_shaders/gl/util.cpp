#include "util.hpp"

namespace Log
{
   static LogFunc logger_cb;

   void set_logger(LogFunc func) { logger_cb = func; }
   LogFunc get_logger() { return logger_cb; }
}

