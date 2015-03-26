/***************************************************************************
 *  navgraph_generator_thread.h - Graph-based global path planning
 *
 *  Created: Tue Sep 18 15:56:35 2012
 *  Copyright  2012  Tim Niemueller [www.niemueller.de]
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

#ifndef __PLUGINS_NAVGRAPH_GENERATOR_NAVGRAPH_GENERATOR_THREAD_H_
#define __PLUGINS_NAVGRAPH_GENERATOR_NAVGRAPH_GENERATOR_THREAD_H_

#ifdef HAVE_VISUALIZATION
#  include "visualization_thread.h"
#endif

#include <core/threading/thread.h>
#include <aspect/configurable.h>
#include <aspect/logging.h>
#include <aspect/blackboard.h>
#include <navgraph/aspect/navgraph.h>
#include <navgraph/navgraph.h>
#include <blackboard/interface_listener.h>
#include <utils/math/types.h>
#include <plugins/amcl/map/map.h>

#include <interfaces/NavGraphGeneratorInterface.h>

#ifdef HAVE_VISUAL_DEBUGGING
#  include <plugins/ros/aspect/ros.h>

namespace ros {
  class Publisher;
}
#endif


class NavGraphGeneratorThread
: public fawkes::Thread,
  public fawkes::LoggingAspect,
  public fawkes::ConfigurableAspect,
  public fawkes::NavGraphAspect,
  public fawkes::BlackBoardAspect,
#ifdef HAVE_VISUAL_DEBUGGING
  public fawkes::ROSAspect,
#endif
  public fawkes::BlackBoardInterfaceListener
{
 public:
  NavGraphGeneratorThread();
  virtual ~NavGraphGeneratorThread();

  virtual void init();
  virtual void loop();
  virtual void finalize();

  /** Stub to see name in backtrace for easier debugging. @see Thread::run() */
 protected: virtual void run() { Thread::run();}

 private:
  typedef struct {
    fawkes::cart_coord_2d_t                            position;
    fawkes::NavGraphGeneratorInterface::ConnectionMode conn_mode;
    std::map<std::string, std::string>                 properties;
  } PointOfInterest;

  typedef std::map<std::string, PointOfInterest>         PoiMap;
  typedef std::map<std::string, fawkes::cart_coord_2d_t> ObstacleMap;

  virtual bool bb_interface_message_received(fawkes::Interface *interface,
                                             fawkes::Message *message) throw();

  ObstacleMap map_obstacles(float line_max_dist);
  map_t * load_map(std::vector<std::pair<int, int>> &free_space_indices);

  void filter_edges_from_map(float max_dist);
  void filter_nodes_orphans();

#ifdef HAVE_VISUAL_DEBUGGING
  void publish_visualization();
#endif

 private:
  std::string  cfg_global_frame_;
  unsigned int cfg_map_line_segm_max_iterations_;
  float        cfg_map_line_segm_distance_threshold_;
  float        cfg_map_line_segm_sample_max_dist_;
  float        cfg_map_line_min_length_;
  unsigned int cfg_map_line_segm_min_inliers_;
  float        cfg_map_line_cluster_tolerance_;
  float        cfg_map_line_cluster_quota_;

  fawkes::NavGraphGeneratorInterface *navgen_if_;

  PoiMap      pois_;
  ObstacleMap obstacles_;
  ObstacleMap map_obstacles_;

  bool                                 copy_default_properties_;
  std::map<std::string, std::string>   default_properties_;

  std::map<std::string, bool>                         filter_;
  std::map<std::string, std::map<std::string, float>> filter_params_float_;
  std::map<std::string, std::map<std::string, float>> filter_params_float_defaults_;

  bool                    bbox_set_;
  fawkes::cart_coord_2d_t bbox_p1_;
  fawkes::cart_coord_2d_t bbox_p2_;

#ifdef HAVE_VISUAL_DEBUGGING
  ros::Publisher *vispub_;
  size_t last_id_num_;
#endif
};

#endif
