#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <map>

class Usd1Parser
{
public:
  enum data_id { Agl, Snr, DATA_MAX };
  static constexpr const char* DATA_NAME[DATA_MAX] = { "agl", "snr" };
  static constexpr data_id DATA_ID[DATA_MAX] = { Agl, Snr };

  struct data_t { uint32_t ts; double val; };

public:
  explicit Usd1Parser();
  virtual ~Usd1Parser();

  bool parse(const std::string& fileName);

  const std::map<std::string, std::string>& info() const { return _info; }
  const std::array<std::vector<data_t>, DATA_MAX>& data() const { return _data; }

private:
  void clearData();
  void addData(data_id id, uint32_t ts, double val);
  void addInfo(const std::string& key, const std::string& val);

  bool parseUsd1(std::ifstream& ifs);

private:
  //log interface

  //log info
  std::map<std::string, std::string> _info;
  //log data
  uint32_t _timestamp;
  std::array<std::vector<data_t>, DATA_MAX> _data;
};
