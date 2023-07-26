#pragma once

#include "sbgEComLib.h"

#include <vector>
#include <string>
#include <map>

class SbgParser
{
public:
  enum data_id { Roll, Pitch, Yaw,
                 Lat, Lon, Alt,
                 Vn, Ve, Vd,
                 Dvx, Dvy, Dvz,
                 Dax, Day, Daz,
                 DATA_MAX };

  struct data_t { uint64_t utc; double val[DATA_MAX]; };
  struct alarm_t { uint64_t ts; uint32_t nb; };

#define DATA_ID_VALID(id) (((id) >= Roll) && ((id) <= Vd))
#define DATA_ID_INVALID(id) (((id) < Roll) || ((id) > Vd))

  static constexpr const char* DATA_NAME[DATA_MAX] = {
    "roll", "pitch", "yaw",
    "lat", "lon", "alt",
    "vn", "ve", "vd",
    "delta_vx", "delta_vy", "delta_vz",
    "delta_ax", "delta_ay", "delta_az"
  };

  static constexpr data_id DATA_ID[DATA_MAX] = {
    Roll, Pitch, Yaw,
    Lat, Lon, Alt,
    Vn, Ve, Vd,
    Dvx, Dvy, Dvz,
    Dax, Day, Daz
  };

public:
  explicit SbgParser();
  virtual ~SbgParser();

  bool open(const std::string& fileName);
  void close();

  const std::map<std::string, std::string>& info() const { return _info; }
  const std::vector<data_t>& data() const { return _data; }

private:
  //callbacks
  friend SbgErrorCode onEComLogReceived(SbgEComHandle*, SbgEComClass msgClass, SbgEComMsgId msg,
                                        const SbgBinaryLogData* pLogData, void* pUserArg);

  void onEComLogUtc(const SbgBinaryLogData* pLogData);
  void onEComLogShort(const SbgBinaryLogData* pLogData);
  void onEComLogImu(const SbgBinaryLogData* pLogData);
  void onEComLogEuler(const SbgBinaryLogData* pLogData);
  void onEComLogQuat(const SbgBinaryLogData* pLogData);
  void onEComLogNav(const SbgBinaryLogData* pLogData);
  void onEComLogGpsVel(const SbgBinaryLogData* pLogData);
  void onEComLogGpsPos(const SbgBinaryLogData* pLogData);
  void onEComLogGpsHdt(const SbgBinaryLogData* pLogData);
  void onEComLogAirData(const SbgBinaryLogData* pLogData);

  void resetUtcTimestamp();

private:
  //sbg interface
  SbgEComHandle _handle;
  SbgInterface _iface;
  //log info
  std::string _fileName;
  std::string _utcStr;
  float _altMin;
  float _altMax;
  float _altDelta;
  std::map<std::string, std::string> _info;
  //log data
  uint32_t _refts;
  uint64_t _refutc;
  bool _initDone;
  std::vector<data_t> _data;
};
