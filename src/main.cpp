/**
 * Copyright (c) 2014 - 2015 Tolga Cakir <tolga@cevel.net>
 *
 * This source file is part of Game Capture HD Linux driver and is distributed
 * under the MIT License. For more information, see LICENSE file.
 */

#include <cstdlib>
#include <iostream>
#include <string>

#include <unistd.h>

#include <gchd.hpp>
#include <process.hpp>
#include <streamer.hpp>

int main(int argc, char *argv[]) {
	// object for managing runtime information
	Process process;

	// object for storing device settings
	Settings settings;

	// commandline-specific settings
	std::string pidPath = "/var/run/gchd.pid";
	std::string outputPath = "/tmp/gchd.ts";
	bool useFifo = true;

	// handling command-line options
	int opt;

	// TODO show help information
	while ((opt = getopt(argc, argv, ":c:di:o:p:r:")) != -1) {
		switch (opt) {
			case 'c':
				if (std::string(optarg) == "yuv") {
					settings.setColorSpace(ColorSpace::YUV);
					break;
				} else if (std::string(optarg) == "rgb") {
					settings.setColorSpace(ColorSpace::RGB);
					break;
				}

				std::cerr << "Unknown colorspace argument.";
			case 'd':
				useFifo = false;
				break;
			case 'i':
				if (std::string(optarg) == "composite") {
					settings.setInputSource(InputSource::Composite);
					break;
				} else if (std::string(optarg) == "component") {
					settings.setInputSource(InputSource::Component);
					break;
				} else if (std::string(optarg) == "hdmi") {
					settings.setInputSource(InputSource::HDMI);
					break;
				}

				std::cerr << "Unknown input source argument." << std::endl;
				return EXIT_FAILURE;
			case 'o':
				outputPath = std::string(optarg);
				break;
			case 'p':
				pidPath = std::string(optarg);
				break;
			case 'r':
				if (std::string(optarg) == "ntsc") {
					settings.setResolution(Resolution::NTSC);
					break;
				} else if (std::string(optarg) == "pal") {
					settings.setResolution(Resolution::PAL);
					break;
				} else if (std::string(optarg) == "720") {
					settings.setResolution(Resolution::HD720);
					break;
				} else if (std::string(optarg) == "1080") {
					settings.setResolution(Resolution::HD1080);
					break;
				}

				std::cerr << "Unknown resolution argument." << std::endl;
				return EXIT_FAILURE;
			case ':':
				std::cerr << "Missing argument." << std::endl;
				return EXIT_FAILURE;
			case '?':
				std::cerr << "Unrecognized option." << std::endl;
				return EXIT_FAILURE;
			default:
				std::cerr << "Unexpected error." << std::endl;
				return EXIT_FAILURE;
		}
	}

	// TODO not ready for primetime yet, program needs to be restarted too
	// often at the moment
	// create PID file for single instance mechanism
//	if (process.createPid(pidPath)) {
//		return EXIT_FAILURE;
//	}

	GCHD gchd(&settings);

	// device initialization
	if(gchd.init()) {
		return EXIT_FAILURE;
	}

	// helper class for streaming audio and video from device
	Streamer streamer(&process);

	if (useFifo) {
		if (streamer.createFifo(outputPath)) {
			return EXIT_FAILURE;
		}

		// when FIFO file has been opened
		streamer.streamToFifo(&gchd);
	} else {
		streamer.streamToDisk(&gchd, outputPath);
	}

	std::cerr << "Terminating." << std::endl;

	return EXIT_SUCCESS;
}
