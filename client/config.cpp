#include "config.hpp"

#include <fstream>

config::config() {
  std::ifstream file("globevision.json");

  if (!file.is_open()) {
	std::ofstream outf("globevision.json");
	outf << _config.dump(2) << std::endl;
	outf.close();
	return;
  }

  const auto temp = nlohmann::json::parse(file);

  for (auto& el : temp.items()) {
	_config[el.key()] = temp[el.key()];
  }

  file.close();
}

config& config::instance() {
  static config inst;
  return inst;
}

size_t config::get_value_size(const char* name) const {
  return _config[name].size();
}
