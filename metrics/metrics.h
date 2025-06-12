//
// Created by Богдан Панов on 13.06.2025.
//

#ifndef METRICS_H
#define METRICS_H

#include <opencv2/core.hpp>

namespace metrics {
	cv::Scalar getMSSIM(const cv::Mat& i1, const cv::Mat& i2);
}

#endif //METRICS_H
