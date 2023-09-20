#pragma once

#include "sbgEComLib.h"

#include <vector>
#include <string>
#include <array>
#include <map>

class SbgParser
{
public:
  enum data_id { Roll, Pitch, Yaw,
                 Lat, Lon, Alt,
                 Vn, Ve, Vd,
                 Dvx, Dvy, Dvz,
                 Dax, Day, Daz,
                 GnssLat, GnssLon, GnssAlt,
                 GnssVn, GnssVe, GnssVd,
                 DATA_MAX };

  struct data_t { uint64_t ts; double val; };

#define DATA_ID_VALID(id) (((id) >= Roll) && ((id) <= Vd))
#define DATA_ID_INVALID(id) (((id) < Roll) || ((id) > Vd))

  static constexpr const char* DATA_NAME[DATA_MAX] = {
    "roll", "pitch", "yaw",
    "lat", "lon", "alt",
    "vn", "ve", "vd",
    "delta_vx", "delta_vy", "delta_vz",
    "delta_ax", "delta_ay", "delta_az",
    "gnss_lat", "gnss_lon", "gnss_alt",
    "gnss_vn", "gnss_ve", "gnss_vd"
  };

  static constexpr data_id DATA_ID[DATA_MAX] = {
    Roll, Pitch, Yaw,
    Lat, Lon, Alt,
    Vn, Ve, Vd,
    Dvx, Dvy, Dvz,
    Dax, Day, Daz,
    GnssLat, GnssLon, GnssAlt,
    GnssVn, GnssVe, GnssVd,
  };

public:
  explicit SbgParser();
  virtual ~SbgParser();

  bool open(const std::string& fileName);
  void close();

  const std::array<std::vector<data_t>, DATA_MAX>& data() const { return _data; }

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

  void clearData();

private:
  //sbg interface
  SbgEComHandle _handle;
  SbgInterface _iface;
  //log info
  std::string _fileName;
  //log data
  uint32_t _utcTimestamp;
  uint32_t _navTimestamp;
  uint32_t _oldTimestamp;
  std::array<std::vector<data_t>, DATA_MAX> _data;
};
