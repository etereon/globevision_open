#pragma once

#include <nlohmann/json.hpp>

class config {
  nlohmann::json _config = {
	{"ip", "127.0.0.1"},
	{"port", 8814},
	{"servers", {}},
  };

  config();

public:

  size_t get_value_size(const char* name) const;

  template<typename Type>
  Type get_value(const char* name) const {
	return _config[name].get<Type>();
  }

  config(const config&) = delete;
  config& operator=(const config&) = delete;

  static config& instance();
};
