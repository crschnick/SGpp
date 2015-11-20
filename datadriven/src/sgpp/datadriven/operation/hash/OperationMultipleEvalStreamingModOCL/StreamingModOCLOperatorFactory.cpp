/*
 * OCLOperatorFactory.hpp
 *
 *  Created on: Mar 25, 2015
 *      Author: pfandedd
 */

#include <sgpp/base/operation/hash/OperationMultipleEval.hpp>
#include <sgpp/base/exception/factory_exception.hpp>
#include <sgpp/globaldef.hpp>
#include <sgpp/datadriven/operation/hash/simple/DatadrivenOperationCommon.hpp>
#include "OperationMultiEvalStreamingModOCL.hpp"

namespace SGPP {
namespace datadriven {

base::OperationMultipleEval* createStreamingModOCLConfigured(base::Grid& grid, base::DataMatrix& dataset,
SGPP::datadriven::OperationMultipleEvalConfiguration &configuration) {

    std::shared_ptr<base::OCLOperationConfiguration> parameters;

    if (configuration.getParameters().operator bool()) {
        base::OCLOperationConfiguration *cloned =
                dynamic_cast<base::OCLOperationConfiguration *>(configuration.getParameters()->clone());
        parameters = std::shared_ptr<base::OCLOperationConfiguration>(cloned);
    } else {
        parameters = std::make_shared<base::OCLOperationConfiguration>("StreamingModOCL.cfg");

        if ((*parameters).contains("KERNEL_USE_LOCAL_MEMORY") == false) {
            (*parameters).addTextAttr("KERNEL_USE_LOCAL_MEMORY", "false");
        }

        if ((*parameters).contains("KERNEL_DATA_BLOCKING_SIZE") == false) {
            (*parameters).addTextAttr("KERNEL_DATA_BLOCKING_SIZE", "1");
        }

        if ((*parameters).contains("LINEAR_LOAD_BALANCING_VERBOSE") == false) {
            (*parameters).addTextAttr("LINEAR_LOAD_BALANCING_VERBOSE", "false");
        }
    }

    if ((*parameters)["VERBOSE"].getBool()) {
        std::cout << "are optimizations on: " << (*parameters)["ENABLE_OPTIMIZATIONS"].getBool() << std::endl;
        std::cout << "is local memory on: " << (*parameters)["KERNEL_USE_LOCAL_MEMORY"].getBool() << std::endl;
        std::cout << "local size: " << (*parameters)["LOCAL_SIZE"].getUInt() << std::endl;
        std::cout << "internal precision: " << (*parameters)["INTERNAL_PRECISION"].get() << std::endl;
        std::cout << "platform is: " << (*parameters)["PLATFORM"].get() << std::endl;
        std::cout << "device type is: " << (*parameters)["DEVICE_TYPE"].get() << std::endl;
    }

    if ((*parameters)["INTERNAL_PRECISION"].get() == "float") {
        return new datadriven::OperationMultiEvalStreamingModOCL<float>(grid, dataset, parameters);
    } else if ((*parameters)["INTERNAL_PRECISION"].get() == "double") {
        return new datadriven::OperationMultiEvalStreamingModOCL<double>(grid, dataset, parameters);
    } else {
        throw base::factory_exception(
                "Error creating operation\"OperationMultiEvalStreamingModOCL\": invalid value for parameter \"INTERNAL_PRECISION\"");
    }
}

}
}