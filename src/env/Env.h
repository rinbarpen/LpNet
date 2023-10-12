#pragma once

#include <yaml-cpp/yaml.h>

class Env
{
public:
  virtual ~Env() = default;
  virtual std::string toYamlString() const = 0;
  virtual std::string type() const = 0;
};
