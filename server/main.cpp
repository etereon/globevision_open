#include "main.hpp"

int main() {
  auto& server = server::instance();

  if (!server.start()) {
	printf("%s - server creation error\n", __FUNCTION__);
	system("pause");
	return 0;
  }  

  server.loop();
  system("pause");
  return 0;
}
