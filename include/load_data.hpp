#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>

struct GasData {
  double nodeFrom;
  double nodeTo;
  double distance;
  double cost;
  double id_from;
  double id_to;
};

struct StationData {
  long nodeFrom;
  long nodeTo;
  long distance;
  long cost;
  long id_from;
  long id_to;
};

const long PREC = 100;

inline void load(std::string fname, std::vector<StationData> &stations) {
  std::vector<GasData> gasData;

  std::ifstream file(fname);

  if (!file.is_open()) {
    std::cout << "Error opening the file." << std::endl;
    return;
  }

  std::string line;
  std::getline(file, line); // Skip the header line

  while (std::getline(file, line)) {
    std::stringstream lineStream(line);
    std::string cell;
    GasData data;

    std::getline(lineStream, cell, ',');
    data.nodeFrom = std::stod(cell);

    std::getline(lineStream, cell, ',');
    data.nodeTo = std::stod(cell);

    std::getline(lineStream, cell, ',');
    data.distance = std::stod(cell);

    std::getline(lineStream, cell, ',');
    data.cost = std::stod(cell);

    std::getline(lineStream, cell, ',');
    data.id_from = std::stod(cell);

    std::getline(lineStream, cell, ',');
    data.id_to = std::stod(cell);

    gasData.push_back(data);
  }

  file.close();
  /*
    CONVERTING ALL THE LONG DOUBLES TO LONG TYPE:
  */
  stations.clear();
  for (auto i : gasData) {
    long nodeFrom = i.nodeFrom;
    long nodeTo = i.nodeTo;
    long distance = i.distance;
    long cost = i.cost * PREC;
    long id_from = i.id_from;
    long id_to = i.id_to;
    stations.push_back({nodeFrom, nodeTo, distance, cost, id_from, id_to});
  }
}
