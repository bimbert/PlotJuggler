#include "usd1_parser.h"

#include <QDebug>

#include <deque>

namespace {

uint16_t extract_uint16_le(const std::deque<uint8_t>& data, uint8_t idx)
{
  uint16_t value = 0;
  value |= (uint16_t)(data[idx+0] << 8);
  value |= (uint16_t)(data[idx+1] << 0);
  return value;
}

uint32_t extract_uint32_le(const std::deque<uint8_t>& data, uint8_t idx)
{
  uint32_t value = 0;
  value |= (uint32_t)(data[idx+0] << 24);
  value |= (uint32_t)(data[idx+1] << 16);
  value |= (uint32_t)(data[idx+2] << 8);
  value |= (uint32_t)(data[idx+3] << 0);
  return value;
}

uint16_t extract_uint16_be(const std::deque<uint8_t>& data, uint8_t idx)
{
  uint16_t value = 0;
  value |= (uint16_t)(data[idx+0] << 0);
  value |= (uint16_t)(data[idx+1] << 8);
  return value;
}

// CRC16 - ITU_T polynomial: x^16 + x^15 + x^2 + 1
static constexpr uint16_t crc16_table[256] = {
  0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
  0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
  0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
  0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
  0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
  0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
  0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
  0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
  0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
  0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
  0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
  0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
  0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
  0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
  0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
  0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
  0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
  0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
  0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
  0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
  0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
  0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
  0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
  0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
  0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
  0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
  0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
  0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
  0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
  0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
  0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
  0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

uint16_t crc16(const std::deque<uint8_t>& data, int size)
{
  uint16_t crc = 0;

  for (int i = 0; i < size; ++i) {
    crc ^= data[i] & 0x00FF;
    crc = crc16_table[crc & 0x00FF] ^ (crc >> 8);
  }

  return crc;
}

}

Usd1Parser::Usd1Parser()
  : _info()
  , _timestamp(0)
  , _data()
{
}

Usd1Parser::~Usd1Parser()
{
}

bool Usd1Parser::parse(const std::string& fileName)
{
  bool ok = false;

  _timestamp = 0;
  clearData();

  std::ifstream ifs(fileName, std::ios::binary | std::ios::in);
  ok = parseUsd1(ifs);

  if (ok) {
    addInfo("session", "full");
  } else {
    addInfo("session", "error");
  }

  return ok;
}

void Usd1Parser::clearData()
{
  std::for_each(_data.begin(), _data.end(), [=](std::vector<data_t>& d){
    d.clear();
  });
}

inline void Usd1Parser::addData(data_id id, uint32_t ts, double val)
{
  _data[id].push_back({ts-_timestamp, val});
}

void Usd1Parser::addInfo(const std::string& key, const std::string& val)
{
  if (_info.find(key) == _info.end()) {
    _info[key] = val;
  }
}

bool Usd1Parser::parseUsd1(std::ifstream& ifs)
{
  static constexpr std::array<uint8_t,2> usd1_log_header = {0x5A,0xA5};
  static constexpr uint8_t usd1_log_size          = 16;       // HDR(2) LEN(2) SBG_TS(4) USD1_MSG(6) CRC(2)
  static constexpr uint8_t usd1_log_payload       = 14;       // Payload size for CRC (HDR LEN SBG_TS USD1_MSG).
  static constexpr uint8_t usd1_log_crc           = 14;       // CRC index
  static constexpr uint8_t usd1_log_timestamp     = 4;        // TIMESTAMP index
  static constexpr uint8_t usd1_log_distance      = 10;       // DISTANCE index (distance in cm)
  static constexpr double usd1_log_data_lsb       = 1.0e-2;   // DATA lsb (cm to m)
  static constexpr uint8_t usd1_log_snr           = 12;       // SNR index

  std::deque<uint8_t> buffer;

  while (ifs.good()) {
    // read some data from input file
    std::string data(usd1_log_size, 0);
    ifs.read(&data[0], usd1_log_size);
    buffer.insert(buffer.end(), data.begin(), data.end());

    // decode all read bytes and publish last decoded value
    bool new_message = false;
    do {
      new_message = false;
      // search for header, remove unnecessary bytes
      auto start = std::search(buffer.begin(), buffer.end(),
                               usd1_log_header.begin(), usd1_log_header.end());
      if (start != buffer.end()) {
        // hear found, clear buffer removing useless bytes from begining
        buffer.erase(buffer.begin(), start);
        start = buffer.begin();

        // if enough bytes remaining, try to decode
        if (buffer.size() >= usd1_log_size) {
          // check if good header with crc
          if (extract_uint16_le(buffer, usd1_log_crc) == crc16(buffer,usd1_log_payload)) {
            // good header, extract message bytes and fill data
            new_message = true;
            uint32_t timestamp = extract_uint32_le(buffer, usd1_log_timestamp);
            uint16_t distance = extract_uint16_be(buffer, usd1_log_distance);
            addData(Agl, timestamp, distance*usd1_log_data_lsb);
            addData(Snr, timestamp, (double)buffer[usd1_log_snr]);
            // remove decoded bytes
            int n = 0;
            do {
              buffer.pop_front();
              ++n;
            } while (!buffer.empty() && (n < usd1_log_size));
          } else {
            // bad header, clear buffer keeping last byte if part of header
            if (buffer[buffer.size()-1] == usd1_log_header[0]) {
              buffer.erase(buffer.begin(), buffer.begin()+buffer.size()-1);
            }
            else {
              buffer.clear();
            }
            addInfo("session", "bad crc");
          }
        }
      }
      else {
        // header not found, clear buffer keeping last byte if part of header
        if (!buffer.empty()) {
          if (buffer[buffer.size()-1] == usd1_log_header[0]) {
            buffer.erase(buffer.begin(), buffer.begin()+buffer.size()-1);
          }
          else {
            buffer.clear();
          }
        }
      }
    } while (new_message);
  }

  return true;
}
