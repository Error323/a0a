/*
  This file is part of Leela Chess Zero.
  Copyright (C) 2018 The LCZero Authors

  Leela Chess is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Leela Chess is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Leela Chess.  If not, see <http://www.gnu.org/licenses/>.

  Additional permission under GNU GPL version 3 section 7

  If you modify this Program, or any covered work, by linking or
  combining it with NVIDIA Corporation's libraries from the NVIDIA CUDA
  Toolkit and the NVIDIA CUDA Deep Neural Network library (or a
  modified version of those libraries), containing parts covered by the
  terms of the respective license agreement, the licensors of this
  Program grant you additional permission to convey the resulting work.
*/

#include "random.h"

#include <glog/logging.h>

#include <chrono>
#include <thread>

namespace utils {

Random::Random() {
  // Mix seeds from different sources to ensure randomness per thread instance
  std::random_device rd;
  std::ranlux48 gen(rd());
  std::uint64_t seed1 = (gen() << 16) ^ gen();
  std::uint64_t seed2 =
      std::chrono::high_resolution_clock::now().time_since_epoch().count();
  std::uint64_t thread_id =
      std::hash<std::thread::id>()(std::this_thread::get_id());
  std::uint64_t seed = seed1 ^ seed2 ^ thread_id;
  rng_.seed(seed);
}

Random& Random::Get() {
  thread_local Random rand;
  return rand;
}

int Random::GetInt(int min, int max) {
  std::uniform_int_distribution<> dist(min, max);
  return dist(rng_);
}

bool Random::GetBool() { return GetInt(0, 1) != 0; }

uint32_t Random::GetFewBits32() {
  std::uniform_int_distribution<uint32_t> dist;
  return dist(rng_) & dist(rng_);
}

double Random::GetDouble(double maxval) {
  std::uniform_real_distribution<> dist(0.0, maxval);
  return dist(rng_);
}

float Random::GetFloat(float maxval) {
  std::uniform_real_distribution<> dist(0.0, maxval);
  return dist(rng_);
}

std::string Random::GetString(int length) {
  std::string result;
  for (int i = 0; i < length; ++i) {
    result += 'a' + GetInt(0, 25);
  }
  return result;
}

double Random::GetGamma(double alpha, double beta) {
  std::gamma_distribution<double> dist(alpha, beta);
  return dist(rng_);
}

}  // namespace utils
