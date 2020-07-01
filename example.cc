
#include <cstdlib>

void example();

int example_argc; char ** example_argv;
int main(int argc, char ** argv) {

  example_argc=argc; example_argv=argv;
  example();
  exit(EXIT_SUCCESS);
}
