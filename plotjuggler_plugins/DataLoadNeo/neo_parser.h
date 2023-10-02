#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <map>

class NeoParser
{
public:
  enum data_id { Agl, DATA_MAX };
  static constexpr const char* DATA_NAME[DATA_MAX] = { "agl" };
  static constexpr data_id DATA_ID[DATA_MAX] = { Agl };

  struct data_t { uint32_t ts; double val; };

public:
  explicit NeoParser();
  virtual ~NeoParser();

  bool parse(const std::string& fileName);

  const std::map<std::string, std::string>& info() const { return _info; }
  const std::array<std::vector<data_t>, DATA_MAX>& data() const { return _data; }

private:
  void clearData();
  void addData(data_id id, uint32_t ts, double val);
  void addInfo(const std::string& key, const std::string& val);

  bool parseNeo(std::ifstream& ifs);

private:
  //log interface

  //log info
  std::map<std::string, std::string> _info;
  //log data
  uint32_t _timestamp;
  std::array<std::vector<data_t>, DATA_MAX> _data;
};
