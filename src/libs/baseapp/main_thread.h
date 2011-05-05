
/***************************************************************************
 *  main_thread.h - Fawkes main thread
 *
 *  Created: Thu Nov  2 16:46:37 2006
 *  Copyright  2006-2008  Tim Niemueller [www.niemueller.de]
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version. A runtime exception applies to
 *  this software (see LICENSE.GPL_WRE file mentioned below for details).
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL_WRE file in the doc directory.
 */

#ifndef __LIBS_BASEAPP_MAIN_THREAD_H_
#define __LIBS_BASEAPP_MAIN_THREAD_H_

#include <core/threading/thread.h>
#include <aspect/mainloop/employer.h>
#include <aspect/logger/employer.h>
#include <utils/system/signal.h>

#include <list>
#include <string>
#include <getopt.h>

namespace fawkes {
#if 0 /* just to make Emacs auto-indent happy */
}
#endif
class BlackBoard;
class Configuration;
class SQLiteConfiguration;
class ConfigNetworkHandler;
class MultiLogger;
class NetworkLogger;
class Clock;
class TimeWait;
class AspectManager;
class PluginManager;
class Time;
class PluginNetworkHandler;
class InterruptibleBarrier;
class Mutex;
class FawkesThreadManager;
class FawkesNetworkManager;

class FawkesMainThread
: public Thread,
  public MainLoopEmployer,
  public LoggerEmployer
{
 public:
  FawkesMainThread(SQLiteConfiguration *config,
		   MultiLogger *multi_logger,
		   BlackBoard *blackboard,
		   const char *load_plugins,
		   unsigned short tcp_port,
		   const char *service_name);
  virtual ~FawkesMainThread();

  virtual void once();
  virtual void loop();

  virtual void set_mainloop_thread(Thread *mainloop_thread);

  virtual void add_logger(Logger *logger);
  virtual void remove_logger(Logger *logger);

  class Runner : public SignalHandler {
  public:
    Runner(FawkesMainThread *fmt);
    ~Runner();
    void run();
    void handle_signal(int signum);
  private:
    FawkesMainThread *__fmt;
    Mutex            *__init_mutex;
    bool              __init_running;
    bool              __init_quit;
    bool              __sigint_running;
  };

 /** Stub to see name in backtrace for easier debugging. @see Thread::run() */
 protected: virtual void run() { Thread::run(); }

 private:
  void destruct();

  Configuration        *__config;
  ConfigNetworkHandler *__config_nethandler;
  BlackBoard           *__blackboard;
  MultiLogger          *__multi_logger;
  NetworkLogger        *__network_logger;
  Clock                *__clock;
  TimeWait             *__time_wait;
  AspectManager        *__aspect_manager;

  Thread               *__mainloop_thread;
  Mutex                *__mainloop_mutex;
  InterruptibleBarrier *__mainloop_barrier;

  char                 *__load_plugins;

  SQLiteConfiguration  *__sqlite_conf;
  FawkesThreadManager  *__thread_manager;
  PluginManager        *__plugin_manager;
  PluginNetworkHandler *__plugin_nethandler;
  Mutex                *__plugin_mutex;
  FawkesNetworkManager         *__network_manager;

  std::list<std::string>        __recovered_threads;
  unsigned int                  __desired_loop_time_usec;
  float                         __desired_loop_time_sec;
  unsigned int                  __max_thread_time_usec;
  unsigned int                  __max_thread_time_nanosec;
  Time                         *__loop_start;
  Time                         *__loop_end;
  bool                          __enable_looptime_warnings;

};

} // end namespace fawkes

#endif
