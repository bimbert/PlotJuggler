#include "zr_parser.h"

#include <QDebug>

#include <deque>

static constexpr uint16_t USD1_LOG_HEADER       = 0x5AA5;
static constexpr uint8_t  USD1_LOG_SIZE         = 16;       // HDR(2) LEN(2) SBG_TS(4) USD1_MSG(6) CRC(2)
static constexpr uint8_t  USD1_LOG_PAYLOAD      = 14;       // Payload size for CRC (HDR LEN SBG_TS USD1_MSG).

ZrParser::ZrParser()
  : _info()
  , _timestamp(0)
  , _data()
{
}

ZrParser::~ZrParser()
{
}

bool ZrParser::parse(const std::string& fileName, type_t type)
{
  bool ok = false;

  _timestamp = 0;
  clearData();

  _type = type;
  addInfo("type", TYPE_NAME[_type]);

  std::ifstream ifs(fileName, std::ios::binary | std::ios::in);
  if (_type == NEO) {
    ok = parseNeo(ifs);
  }
  else if (_type == USD1) {
    ok = parseUsd1(ifs);
  }

  if (ok) {
    addInfo("session", "full");
  } else {
    addInfo("session", "error");
  }

  return ok;
}

void ZrParser::clearData()
{
  std::for_each(_data.begin(), _data.end(), [=](std::vector<data_t>& d){
    d.clear();
  });
}

inline void ZrParser::addData(data_id id, uint32_t ts, double val)
{
  _data[id].push_back({ts-_timestamp, val});
}

void ZrParser::addInfo(const std::string& key, const std::string& val)
{
  if (_info.find(key) == _info.end()) {
    _info[key] = val;
  }
}

bool ZrParser::parseNeo(std::ifstream& ifs)
{
  static constexpr std::array<uint8_t,2> neo_log_header = {0x5A,0xA5};
  static constexpr uint8_t neo_log_size           = 16; // HDR(2) LEN(2) SBG_TS(4) NEO_MSG(5) PAD(1) CRC(2)
  static constexpr uint8_t NEO_LOG_PAYLOAD        = 14;       // Payload size for CRC (HDR LEN SBG_TS NEO_MSG PAD).
  static constexpr uint8_t teraranger_neo_header  = 0x54;     // Binary header 0x54 ('T').

  std::deque<uint8_t> buffer;

  while (ifs.good()) {
    // read some data from input file
    std::string data(neo_log_size, 0);
    ifs.read(&data[0], neo_log_size);
    buffer.insert(buffer.end(), data.begin(), data.end());

    // decode all read bytes and publish last decoded value
    bool new_message = false;
    do {
      new_message = false;
      // search for header, remove unnecessary bytes
      auto start = std::search(buffer.begin(), buffer.end(),
                               neo_log_header.begin(), neo_log_header.end());
      if (start != buffer.end()) {
        // hear found, clear buffer removing useless bytes from begining
        buffer.erase(buffer.begin(), start);
        start = buffer.begin();

        // if enough bytes remaining, try to decode
        if (buffer.size() >= neo_log_size) {
          // check if good header byte with crc
          if (buffer[TERARANGER_NEO_CRC] == crc8(buffer,TERARANGER_NEO_PAYLOAD)) {
            // good header byte, extract message bytes and noitfy
            new_message = true;
            formatMessage();
            ready_(message_);
            // remove decoded bytes
            int n = 0;
            do {
              buffer.pop_front();
              ++n;
            } while (!buffer.empty() && (n < TERARANGER_NEO_SIZE));
            const time_point<steady_clock> now = steady_clock::now();
            if ((now - trace_) >= 1s) {
              trace_ = now;
              DEBUG("NeoProcessor::decode()","remaining[%d]", buffer.size());
            }
          } else {
            // bad header byte, go to next header if any
            do {
              buffer.pop_front();
            } while (!buffer.empty() && (buffer.front() != TERARANGER_NEO_HEADER));
            DEBUG("NeoProcessor::decode()","Bad crc[%02X/%02X]",
                  buffer[TERARANGER_NEO_CRC], crc8(buffer,TERARANGER_NEO_PAYLOAD));
          }
        }
      }
      else {
        // header not found, clear buffer keeping last byte if part of header
        if (buffer[buffer.size()-1] == neo_log_header[0]) {
          buffer.erase(buffer.begin(), buffer.begin()+buffer.size()-1);
        }
        else {
          buffer.clear();
        }
      }

      while (!buffer.empty() && (buffer.front() != TERARANGER_NEO_HEADER)) {
        buffer.pop_front();
      }
      // if header found and enough bytes, try to decode
      if (buffer.size() >= TERARANGER_NEO_SIZE) {

      }
    } while (new_message);
  }
}

bool ZrParser::parseUsd1(std::ifstream& ifs)
{

}
