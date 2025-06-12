//
// Created by Богдан Панов on 13.06.2025.
//

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

namespace metrics {
	// ------------------------------ SSIM ---------------------------------------
	// A compact multichannel SSIM implementation (Wang et al., 2004).
	// ---------------------------------------------------------------------------
	Scalar getMSSIM(const Mat& i1, const Mat& i2) {
		const double C1 = 6.5025, C2 = 58.5225;
		Mat I1, I2;
		i1.convertTo(I1, CV_32F);
		i2.convertTo(I2, CV_32F);

		Mat I1_2 = I1.mul(I1); // I1^2
		Mat I2_2 = I2.mul(I2); // I2^2
		Mat I1_I2 = I1.mul(I2); // I1*I2

		Mat mu1, mu2;
		GaussianBlur(I1, mu1, Size(11, 11), 1.5);
		GaussianBlur(I2, mu2, Size(11, 11), 1.5);

		Mat mu1_2 = mu1.mul(mu1);
		Mat mu2_2 = mu2.mul(mu2);
		Mat mu1_mu2 = mu1.mul(mu2);

		Mat sigma1_2, sigma2_2, sigma12;
		GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
		sigma1_2 -= mu1_2;
		GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
		sigma2_2 -= mu2_2;
		GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
		sigma12 -= mu1_mu2;

		Mat t1, t2, t3;
		t1 = 2 * mu1_mu2 + C1;
		t2 = 2 * sigma12 + C2;
		t3 = t1.mul(t2); // numerator

		t1 = mu1_2 + mu2_2 + C1;
		t2 = sigma1_2 + sigma2_2 + C2;
		t1 = t1.mul(t2); // denominator

		Mat ssim_map;
		divide(t3, t1, ssim_map);
		return mean(ssim_map);
	}
} // namespace metrics
