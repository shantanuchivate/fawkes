
/***************************************************************************
 *  feature_config.cpp -  CLIPS config feature
 *
 *  Created: Sun Oct 06 13:08:00 2013
 *  Copyright  2006-2013  Tim Niemueller [www.niemueller.de]
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include "feature_config.h"
#include <config/config.h>
#include <logging/logger.h>

#include <clipsmm.h>

using namespace fawkes;

/** @class ConfigCLIPSFeature "feature_blackboard.h"
 * CLIPS blackboard feature.
 * @author Tim Niemueller
 */

/** Constructor.
 * @param logger message logger
 * @param config configuration to use for CLIPS commands
 */
ConfigCLIPSFeature::ConfigCLIPSFeature(fawkes::Logger *logger,
				       fawkes::Configuration *config)
: CLIPSFeature("config"), logger_(logger), config_(config)
{
}


/** Destructor. */
ConfigCLIPSFeature::~ConfigCLIPSFeature()
{
  envs_.clear();
}


void
ConfigCLIPSFeature::clips_context_init(const std::string &env_name,
				       fawkes::LockPtr<CLIPS::Environment> &clips)
{
  envs_[env_name] = clips;
  clips->evaluate("(path-load \"ff-config.clp\")");
  clips->add_function("config-load",
    sigc::slot<void, std::string>(
      sigc::bind<0>(
        sigc::mem_fun(*this, &ConfigCLIPSFeature::clips_config_load),
        env_name)
    )
  );
}

void
ConfigCLIPSFeature::clips_context_destroyed(const std::string &env_name)
{
  envs_.erase(env_name);
}


void
ConfigCLIPSFeature::clips_config_load(std::string env_name, std::string cfg_prefix)
{
  std::string name = "ClipsConfig|" + env_name;

  if (envs_.find(env_name) == envs_.end()) {
    logger_->log_warn(name.c_str(), "Environment %s has not been registered "
		     "for config feature", env_name.c_str());
    return;
  }

  std::auto_ptr<Configuration::ValueIterator> v(config_->search(cfg_prefix.c_str()));
  while (v->next()) {
    std::string type = "";
    std::string value = v->get_as_string();

    if      (v->is_uint())   type = "UINT";
    else if (v->is_int())    type = "INT";
    else if (v->is_float())  type = "FLOAT";
    else if (v->is_bool())   type = "BOOL";
    else if (v->is_string()) {
      type = "STRING";
      if (! v->is_list()) {
	value = std::string("\"") + value + "\"";
      }
    } else {
      logger_->log_warn(name.c_str(), "Config value at '%s' of unknown type '%s'",
		       v->path(), v->type());
    }

    if (v->is_list()) {
      envs_[env_name]->assert_fact_f("(confval (path \"%s\") (type %s) "
				     "(is-list TRUE) (list-value %s))",
				     v->path(), type.c_str(), value.c_str());
    } else {
      envs_[env_name]->assert_fact_f("(confval (path \"%s\") (type %s) (value %s))",
				     v->path(), type.c_str(), value.c_str());
    }

  }
}
