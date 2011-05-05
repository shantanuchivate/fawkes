
/***************************************************************************
 *  run.cpp - Fawkes run functions
 *
 *  Created: Wed May 04 23:23:23 2011
 *  Copyright  2006-2011  Tim Niemueller [www.niemueller.de]
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

#include <baseapp/run.h>
#include <baseapp/daemonize.h>
#include <baseapp/main_thread.h>

#include <core/threading/thread.h>

#include <blackboard/local.h>
#include <config/sqlite.h>
#include <utils/ipc/shm.h>
#include <utils/system/argparser.h>
#include <utils/logging/multi.h>
#include <utils/logging/console.h>
#include <utils/logging/liblogger.h>
#include <utils/logging/factory.h>
#include <utils/time/clock.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace fawkes {
  namespace runtime {
#if 0 /* just to make Emacs auto-indent happy */
  }
}
#endif

ArgumentParser       *argument_parser = NULL;
FawkesMainThread     *main_thread = NULL;
MultiLogger          *logger = NULL;
BlackBoard           *blackboard = NULL;
SQLiteConfiguration  *config = NULL;


int
init(int argc, char **argv)
{
  // *** parse arguments

  option long_options[] = {
    {"net-service-name", 1, 0, 0},
      {0, 0, 0, 0}
  };
  
  argument_parser = new ArgumentParser(argc, argv,
				       "hCc:d:q::l:L:p:P:u:g:D::ks",
				       long_options);

  if ( argument_parser->has_arg("D") ) {
    fawkes::daemon::init(argument_parser->arg("D"), argv[0]);
    if (argument_parser->has_arg("k")) {
      fawkes::daemon::kill();
    } else if (argument_parser->has_arg("s")) {
      return fawkes::daemon::running() ? 0 : 1;
    } else {
      fawkes::daemon::start();
    }
  }

  // *** set user group if requested
  const char *user  = NULL;
  const char *group = NULL;
  if (argument_parser->has_arg("u")) {
    user = argument_parser->arg("u");
  }
  if (argument_parser->has_arg("g")) {
    group = argument_parser->arg("g");
  }

  if (user != NULL) {
    struct passwd *pw;
    if (! (pw = getpwnam(user))) {
      printf("Failed to find user %s, check -u argument.\n", user);
      return 203;
    }
    int r = 0;
    r = setreuid(pw->pw_uid, pw->pw_uid);
    if (r < 0) {
      perror("Failed to drop privileges (user)");
    }
  }

  if (group != NULL) {
    struct group *gr;
    if (! (gr = getgrnam(group))) {
      printf("Failed to find group %s, check -g argument.\n", user);
      return 204;
    }
    int r = 0;
    r = setregid(gr->gr_gid, gr->gr_gid);
    if (r < 0) {
      perror("Failed to drop privileges (group)");
    }
  }

  // *** setup base thread and shm registry
  Thread::init_main();
  SharedMemoryRegistry shm_registry(true);

  // *** setup logging
  const char *tmp;
  Logger::LogLevel log_level = Logger::LL_DEBUG;
  if ( argument_parser->has_arg("q") ) {
    log_level = Logger::LL_INFO;
    if ( (tmp = argument_parser->arg("q")) != NULL ) {
      for (unsigned int i = 0; i < strlen(tmp); ++i) {
	if ( tmp[i] == 'q' ) {
	  switch (log_level) {
	  case Logger::LL_INFO:  log_level = Logger::LL_WARN; break;
	  case Logger::LL_WARN:  log_level = Logger::LL_ERROR; break;
	  case Logger::LL_ERROR: log_level = Logger::LL_NONE; break;
	  default: break;
	  }
	}
      }
    }
  } else if ( (tmp = argument_parser->arg("l")) != NULL ) {
    if ( strcmp(tmp, "debug") == 0 ) {
      log_level = Logger::LL_DEBUG;
    } else if ( strcmp(tmp, "info") == 0 ) {
      log_level = Logger::LL_INFO;
    } else if ( strcmp(tmp, "warn") == 0 ) {
      log_level = Logger::LL_WARN;
    } else if ( strcmp(tmp, "error") == 0 ) {
      log_level = Logger::LL_ERROR;
    } else if ( strcmp(tmp, "none") == 0 ) {
      log_level = Logger::LL_NONE;
    } else {
      printf("Unknown log level '%s', using default\n", tmp);
    }
  }

  if ( argument_parser->has_arg("L") != 0 ) {
    try {
      logger = LoggerFactory::multilogger_instance(argument_parser->arg("L"));
    } catch (Exception &e) {
      e.append("Initializing multi logger failed");
      throw;
    }
  } else {
    logger = new MultiLogger(new ConsoleLogger());
  }

  logger->set_loglevel(log_level);
  LibLogger::init(logger);

  // *** Prepare home dir directory, just in case
  const char *homedir = getenv("HOME");
  if (homedir) {
    char *userdir;
    if (asprintf(&userdir, "%s/%s", homedir, USERDIR) != -1) {
      if (access(userdir, W_OK) != 0) {
	if (mkdir(userdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
	  logger->log_warn("FawkesMainThread", "Failed to create .fawkes "
			   "directory %s, trying without", userdir);
	}
      }
      free(userdir);
    }
  }

  // *** setup config
  config = new SQLiteConfiguration(CONFDIR);

  config->load(argument_parser->arg("c"), argument_parser->arg("d"));

  try {
    SQLiteConfiguration::SQLiteValueIterator *i = config->modified_iterator();
    while (i->next()) {
      std::string modtype = i->get_modtype();
      if (modtype == "changed") {
	logger->log_warn("FawkesMainThread", "Default config value CHANGED: %s"
			 "(was: %s now: %s)", i->path(),
			 i->get_oldvalue().c_str(), i->get_as_string().c_str());
      } else if (modtype == "erased") {
	logger->log_warn("FawkesMainThread", "Default config value ERASED:  %s",
			 i->path());
      } else {
	logger->log_debug("FawkesMainThread", "Default config value ADDED:   %s "
			  "(value: %s)", i->path(), i->get_as_string().c_str());
      }
    }
    delete i;
  } catch (Exception &e) {
    logger->log_warn("FawkesMainThread", "Failed to read modified default "
		     "config values, no dump?");
  }


  // *** Determine network parameters
  unsigned int net_tcp_port     = 1910;
  std::string  net_service_name = "Fawkes on %h";
  if (argument_parser->has_arg("P")) {
    try {
      net_tcp_port = argument_parser->parse_int("P");
    } catch (Exception &e) {
      logger->log_warn("FawkesMainThread", "Illegal port '%s', using %u",
		       argument_parser->arg("P"), net_tcp_port);
    }
  } else {
    try {
      net_tcp_port = config->get_uint("/fawkes/mainapp/net/tcp_port");
    } catch (Exception &e) {}  // ignore, we stick with the default
  }

  if (argument_parser->has_arg("net-service-name")) {
    net_service_name = argument_parser->arg("net-service-name");
  } else {
    try {
      net_service_name = config->get_string("/fawkes/mainapp/net/service_name");
    } catch (Exception &e) {}  // ignore, we stick with the default
  }

  if (net_tcp_port > 65535) {
    logger->log_warn("FawkesMainThread", "Invalid port '%u', using 1910",
		     net_tcp_port);
    net_tcp_port = 1910;
  }

  // *** Setup blackboard
  std::string bb_magic_token = "";
  unsigned int bb_size = 2097152;
  try {
    bb_magic_token = config->get_string("/fawkes/mainapp/blackboard_magic_token");
    logger->log_info("FawkesMainApp", "BlackBoard magic token defined. "
		     "Using shared memory BlackBoard.");
  } catch (Exception &e) {
    // ignore
  }
  try {
    bb_size = config->get_uint("/fawkes/mainapp/blackboard_size");
  } catch (Exception &e) {
    logger->log_warn("FawkesMainApp", "BlackBoard size not defined. "
		     "Will use %u, saving to default DB", bb_size);
    config->set_default_uint("/fawkes/mainapp/blackboard_size", bb_size);
  }

  // Cleanup stale BlackBoard shared memory segments if requested
  if ( argument_parser->has_arg("C") ) {
    LocalBlackBoard::cleanup(bb_magic_token.c_str(),
			     /* output with lister? */ true);
  }

  if ( bb_magic_token == "") {
    blackboard       = new LocalBlackBoard(bb_size);
  } else {
    blackboard       = new LocalBlackBoard(bb_size, bb_magic_token.c_str());
  }

  // *** Create main thread, but do not start, yet
  main_thread = new fawkes::FawkesMainThread(config, logger, blackboard,
					     argument_parser->arg("p"),
					     net_tcp_port,
					     net_service_name.c_str());


  return 0;
}

void
cleanup()
{
  Thread::destroy_main();

  if ( argument_parser->has_arg("D") ) {
    fawkes::daemon::cleanup();
  }

  delete main_thread;
  delete argument_parser;

  main_thread = NULL;
  argument_parser = NULL;

  // implicitly frees multi_logger and all sub-loggers
  LibLogger::finalize();

  logger = NULL;

  Clock::finalize();
}

void
run()
{
  FawkesMainThread::Runner fawkes(main_thread);
  SignalManager::register_handler(SIGINT, &fawkes);
  SignalManager::register_handler(SIGTERM, &fawkes);

  try {
    fawkes.run();
  } catch (Exception &e) {
    printf("Running Fawkes failed\n");
    e.print_trace();
  }
}


void
print_usage(const char *progname)
{
  printf("Fawkes Main Application - Usage Instructions\n"
	 "================================================"
	 "===============================\n"
	 "Usage: %s [options]\n"
	 "where [options] is one or more of:\n"
         " -h                       These help instructions\n"
         " -C                       Cleanup old BB segments\n"
         " -c db-file               Mutable configuration file, created if it "
	 "does not\n                          "
	 "exist, if it does must contain valid SQLite database\n"
         " -d sql-file              Default configuration SQL dump file.\n"
         " -q[qqq]                  Quiet mode, -q omits debug, -qq debug and"
	 "info,\n                          "
	 "-qqq omit debug, info and warn, -qqqq no output\n"
         " -l level                 Set log level directly mutually exclusive"
	 "with -q,\n                          "
	 "level is one of debug, info, warn, error, or none\n"
         " -L loggers               Define loggers. By default this setting is"
	 "read from\n                          "
	 "config (console logger if unset). Format is:\n"
	 "                          logger:args[;logger2:args2[!...]]\n"
	 "                          Currently supported:\n"
         "                          console, file:file.log, network logger always added\n"
         " -p plugins               List of plugins to load on startup in given order\n"
         " -P port                  TCP port to listen on for Fawkes network connections.\n"
         " --net-service-name=name  mDNS service name to use.\n"
	 " -u user                  Drop privileges and run as given user.\n"
         " -g group                 Drop privileges and run as given group.\n"
#ifdef HAVE_LIBDAEMON
         " -D[pid file]             Run daemonized in the background, pid file "
	 "is optional,\n                          "
	 "default is /var/run/fawkes.pid, must be absolute path.\n"
         " -D[pid file] -k          Kill a daemonized Fawkes running in the"
	 "background\n"
         " -D[pid file] -s          Check status of daemon.\n"
#endif
	 "\n", progname);
}


} // end namespace runtime
} // end namespace fawkes
