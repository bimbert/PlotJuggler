#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <map>

class ZrParser
{
public:
  enum type_t { NEO, USD1, TYPE_MAX };
  static constexpr const char* TYPE_NAME[TYPE_MAX] = { "neo", "usd1" };
  static constexpr type_t TYPE[TYPE_MAX] = { NEO, USD1 };

  enum data_id { Agl, DATA_MAX };
  static constexpr const char* DATA_NAME[DATA_MAX] = { "agl" };
  static constexpr data_id DATA_ID[DATA_MAX] = { Agl };

  struct data_t { uint32_t ts; double val; };

public:
  explicit ZrParser();
  virtual ~ZrParser();

  bool parse(const std::string& fileName, type_t type);

  const std::map<std::string, std::string>& info() const { return _info; }
  const std::array<std::vector<data_t>, DATA_MAX>& data() const { return _data; }

private:
  void clearData();
  void addData(data_id id, uint32_t ts, double val);
  void addInfo(const std::string& key, const std::string& val);

  bool parseNeo(std::ifstream& ifs);
  bool parseUsd1(std::ifstream& ifs);

private:
  //log interface
  type_t _type;
  //log info
  std::map<std::string, std::string> _info;
  //log data
  uint32_t _timestamp;
  std::array<std::vector<data_t>, DATA_MAX> _data;
};
