#include <cstdio>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  std::printf(
      "okinawa-docs: usage: okinawa-docs --in docs/content --out docs/dist "
      "[--base-url /okinawa.cpp] [--template docs/templates/layout.html] "
      "[--static docs/static]\n");
  return 0;
}
