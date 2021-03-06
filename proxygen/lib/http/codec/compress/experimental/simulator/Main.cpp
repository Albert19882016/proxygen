/*
 *  Copyright (c) 2017-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include <folly/portability/GFlags.h>
#include <folly/init/Init.h>

#include "proxygen/lib/http/codec/compress/experimental/simulator/CompressionSimulator.h"
#include <proxygen/lib/http/codec/compress/HPACKHeader.h>
#include <proxygen/lib/http/codec/compress/HPACKEncoder.h>


DEFINE_string(input, "", "File containing requests");
DEFINE_string(scheme, "qpack", "Scheme: <qpack|qcram|hpack>");

DEFINE_int32(rtt, 100, "Simulated RTT");
DEFINE_double(lossp, 0.0, "Loss Probability");
DEFINE_double(delayp, 0.05, "Delay Probability");
DEFINE_int32(delay, 100, "Max extra delay");
DEFINE_int32(ooo_thresh, 0, "First seqn to allow ooo");
DEFINE_int64(seed, 0, "RNG seed");
DEFINE_bool(blend, true, "Blend all facebook.com and fbcdn.net domains");
DEFINE_bool(same_packet_compression, true, "Allow QCRAM to compress across "
            "headers the same packet");

using namespace proxygen::compress;

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv, true);
  if (FLAGS_same_packet_compression) {
    proxygen::HPACKEncoder::enableAutoFlush();
  }

  if (FLAGS_input.empty()) {
    LOG(ERROR) << "Must supply a filename";
    return 1;
  }

  SchemeType t = SchemeType::QCRAM;
  if (FLAGS_scheme == "qcram") {
    LOG(INFO) << "Using QCRAM";
    t = SchemeType::QCRAM;
  } else if (FLAGS_scheme == "qpack") {
    LOG(INFO) << "Using QPACK";
    t = SchemeType::QPACK;
  } else if (FLAGS_scheme == "hpack") {
    LOG(INFO) << "Using HPACK";
    t = SchemeType::HPACK;
  } else {
    LOG(ERROR) << "Unsupported scheme";
    return 1;
  }

  if (FLAGS_seed == 0) {
    FLAGS_seed = folly::Random::rand64();
  }
  SimParams p{t, FLAGS_seed, std::chrono::milliseconds(FLAGS_rtt), FLAGS_lossp,
      FLAGS_delayp, std::chrono::milliseconds(FLAGS_delay),
      uint16_t(FLAGS_ooo_thresh), FLAGS_blend, FLAGS_same_packet_compression};
  CompressionSimulator sim(p);
  if (sim.readInputFromFileAndSchedule(FLAGS_input)) {
    sim.run();
  }

  return 0;
}
