#pragma once

#include "testing/RegularityWitness.hpp"

#include <gtest/gtest.h>

#include <vector>


namespace mpc::detail {

template <std::size_t pad_size>
struct Pad {
  char pad[pad_size];
};

template <>
struct Pad<0> {};


using ValueLog = std::vector<unsigned int>;

template <mpc::RegularityPolicy options, std::size_t pad_size>
class Logger : public mpc::RegularityWitness<options>, private Pad<pad_size> {
private:
  ValueLog* got_;

public:
  explicit Logger(ValueLog* got)
    : got_(got) {}

  void operator()(unsigned int num) {
    got_->push_back(num);
  }
};

template <mpc::RegularityPolicy options, std::size_t pad_size>
class LoggerChecker {
public:
  Logger<options, pad_size> getLogger() {
    return Logger<options, pad_size>{&got_};
  }

  ValueLog pollValues() {
    auto result = got_;
    got_.clear();
    return result;
  }

private:
  ValueLog got_;
};

template <mpc::RegularityPolicy options>
struct Counter : public mpc::RegularityWitness<options> {
  int x = 0;
  bool isPositive() const {
    return x > 0;
  }

  Counter(int x) :x(x) {}
  Counter() {}
};

}
