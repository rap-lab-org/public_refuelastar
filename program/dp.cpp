#include <string>
#include <vector>
#include "load_data.hpp"

void solve(const std::vector<StationData>& stations, const long s, const long t, const long K, const long Q) {

}

int main(int argc, char **argv) {
  std::string file = std::string(argv[1]);
  long s = std::stoi(argv[2]);
  long t = std::stoi(argv[3]);
  long K = std::stoi(argv[4]);
  long Q = std::stoi(argv[5]);
  std::vector<StationData> stations;
  load(file, stations);
  solve(stations, s, t, K, Q);
}
