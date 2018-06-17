/* Copyright (C) 2008-today The SG++ project
 *
 * This file is part of the SG++ project. For conditions of distribution and
 * use, please see the copyright notice provided with SG++ or at
 * sgpp.sparsegrids.org
 *
 * ScorerMetricTypeParser.hpp
 *
 *  Created on: 21.12.2016
 *      Author: Michael Lettrich
 */

#pragma once

#include <sgpp/datadriven/datamining/modules/scoring/ScorerConfig.hpp>

#include <map>
#include <string>

namespace sgpp {
namespace datadriven {

/**
 * Convenience class to convert strings to #sgpp::datadriven::ScorerMetricType and generate
 * string representations for values of #sgpp::datadriven::ScorerMetricType.
 */
class ScorerMetricTypeParser {
 public:
  /**
   * Convert strings to values #sgpp::datadriven::ScorerMetricType. Throws if there is no valid
   * representation
   * @param input case insensitive string representation of a
   * #sgpp::datadriven::ScorerMetricType.
   * @return the corresponding #sgpp::datadriven::ScorerMetricType.
   */
  static ScorerMetricType parse(const std::string &input);

  /**
   * generate string representations for values of #sgpp::datadriven::ScorerMetricType.
   * @param type enum value.
   * @return string representation of a #sgpp::datadriven::ScorerMetricType.
   */
  static const std::string &toString(ScorerMetricType type);

 private:
  typedef std::map<ScorerMetricType, std::string> MetricTypeMap_t;
  /**
   * Map containing all values of  #sgpp::datadriven::ScorerMetricType and the corresponding
   * string representation.
   */
  static const MetricTypeMap_t metricTypeMap;
};

} /* namespace datadriven */
} /* namespace sgpp */
