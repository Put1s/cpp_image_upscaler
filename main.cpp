/*
 * Super‑resolution demo with optional quality metrics
 * ---------------------------------------------------
 * By default the program simply upscales a low‑resolution image using the
 * selected algorithm (exactly as the classic OpenCV example). If you pass the
 * flag --metrics (or -m), the program switches to benchmark mode: it treats the
 * supplied image as the high‑resolution ground truth, downsamples it, restores
 * it and then reports PSNR and SSIM.
 *
 * Usage (simple upscale):
 *   ./superres <image> <algorithm> <scale> [model_path]
 *
 * Usage (with metrics):
 *   ./superres <image> <algorithm> <scale> [model_path] --metrics
 *
 *   <image>      – path to the image file
 *   <algorithm>  – bilinear | bicubic | edsr | espcn | fsrcnn | lapsrn
 *   <scale>      – 2 | 3 | 4 | 8  (LapSRN supports 8x; interp. works for any)
 *   [model_path] – required only for DNN algorithms (edsr / espcn / fsrcnn / lapsrn)
 *   --metrics    – optional flag to compute PSNR & SSIM (implies synthetic LR)
 *
 * Example (plain upscale):
 *   ./superres low.png bicubic 3
 *
 * Example (metrics):
 *   ./superres lena.png edsr 4 EDSR_x4.pb --metrics
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn_superres.hpp>

#include "metrics/metrics.h"

using namespace std;


// ------------------------------- helpers -----------------------------------
static void printUsage() {
	cerr << "Usage (upscale):\n" << "  ./superres <image> <algorithm> <scale> [model_path]\n\n" <<
		"Usage (with metrics):\n" << "  ./superres <image> <algorithm> <scale> [model_path] --metrics\n\n" <<
		"Algorithms: bilinear | bicubic | edsr | espcn | fsrcnn | lapsrn\n";
}

static bool isDnnAlgo(const string& alg) {
	return alg == "edsr" || alg == "espcn" || alg == "fsrcnn" || alg == "lapsrn";
}

// --------------------------------- MAIN ------------------------------------
int main(int argc, char** argv) {
	if (argc < 4) {
		printUsage();
		return EXIT_FAILURE;
	}

	// -------------------------------------------------------------------
	// 1. Parse arguments & flag
	// -------------------------------------------------------------------
	bool withMetrics = false;
	bool quiet = false;
	vector<string> args;
	for (int i = 1; i < argc; ++i) {
		string a(argv[i]);
		if (a == "--metrics" || a == "-m")
			withMetrics = true;
		else if (a == "--quiet" || a == "-q")
			quiet = true;
		else
			args.push_back(a);
	}

	if (args.size() < 3) {
		printUsage();
		return EXIT_FAILURE;
	}

	const string imgPath = args[0];
	const string algorithm = args[1];
	const int scale = stoi(args[2]);
	const string modelPath = (isDnnAlgo(algorithm) && args.size() >= 4) ? args[3] : "";

	if (isDnnAlgo(algorithm) && modelPath.empty()) {
		cerr << "Error: model path required for DNN algorithm '" << algorithm << "'." << endl;
		return EXIT_FAILURE;
	}

	// -------------------------------------------------------------------
	// 2. Load image
	// -------------------------------------------------------------------
	cv::Mat img = imread(imgPath, cv::IMREAD_COLOR);
	if (img.empty()) {
		cerr << "Error: cannot open image at " << imgPath << endl;
		return EXIT_FAILURE;
	}

	cv::Mat imgInput, imgRestored, imgGT;

	if (withMetrics) {
		// Use the loaded image as ground truth (GT), build synthetic LR input
		imgGT = img.clone();
		resize(imgGT, imgInput, cv::Size(), 1.0 / scale, 1.0 / scale, cv::INTER_AREA);
	} else {
		imgInput = img; // Treat the loaded image as low‑res input
	}

	// -------------------------------------------------------------------
	// 3. Upscale
	// -------------------------------------------------------------------
	const auto tStart = chrono::steady_clock::now();

	if (algorithm == "bilinear") {
		resize(imgInput, imgRestored, cv::Size(), scale, scale, cv::INTER_LINEAR);
	} else if (algorithm == "bicubic") {
		resize(imgInput, imgRestored, cv::Size(), scale, scale, cv::INTER_CUBIC);
	} else if (isDnnAlgo(algorithm)) {
		cv::dnn_superres::DnnSuperResImpl sr;
		try {
			sr.readModel(modelPath);
			sr.setModel(algorithm, scale);
			sr.upsample(imgInput, imgRestored);
		} catch (const cv::Exception& e) {
			cerr << "Error during DNN upsampling: " << e.what() << endl;
			return EXIT_FAILURE;
		}
	} else {
		cerr << "Error: unknown algorithm '" << algorithm << "'." << endl;
		return EXIT_FAILURE;
	}

	const auto tEnd = chrono::steady_clock::now();
	const double elapsedMs = static_cast<double>(chrono::duration_cast<chrono::milliseconds>(tEnd - tStart).count());

	// -------------------------------------------------------------------
	// 4. Metrics (optional)
	// -------------------------------------------------------------------
	double psnr = 0.0;
	cv::Scalar ssim;
	if (withMetrics) {
		// Ensure sizes match (some models output slightly different dims)
		if (imgRestored.size() != imgGT.size())
			resize(imgRestored, imgRestored, imgGT.size(), 0, 0, cv::INTER_CUBIC);
		psnr = cv::PSNR(imgGT, imgRestored);
		ssim = metrics::getMSSIM(imgGT, imgRestored);
	}

	// -------------------------------------------------------------------
	// 5. Report
	// -------------------------------------------------------------------
	cout << fixed << setprecision(5);
	if (quiet) {
		if (withMetrics)
			cout << psnr << ' ' << ssim[0] << ' ' << ssim[1] << ' ' << ssim[2] << endl;
	} else {
		cout << "\n==== Results ===============================" << endl;
		cout << "Mode        : " << (withMetrics ? "metrics" : "upscale-only") << endl;
		cout << "Algorithm   : " << algorithm << endl;
		cout << "Scale       : " << scale << "x" << endl;
		cout << "Time        : " << elapsedMs << " ms" << endl;
		if (withMetrics) {
			cout << "PSNR        : " << psnr << " dB" << endl;
			cout << "SSIM (B,G,R): " << ssim[0] << ", " << ssim[1] << ", " << ssim[2] << endl;
		}
		cout << "===========================================\n" << endl;
	}

	// -------------------------------------------------------------------
	// 6. Save & visualisation
	// -------------------------------------------------------------------


	if (!quiet) {
		string outName = withMetrics ? "restored_" + algorithm + "_m.png" : "restored_" + algorithm + ".png";
		imwrite(outName, imgRestored);
		cout << "Restored image saved as " << outName << endl;
		if (withMetrics)
			imshow("Ground Truth", imgGT);
		imshow("Input", imgInput);
		imshow("Restored (" + algorithm + ")", imgRestored);
		cv::waitKey(0);
	}
	return EXIT_SUCCESS;
}
