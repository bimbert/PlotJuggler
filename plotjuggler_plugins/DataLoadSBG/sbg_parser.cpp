#include "sbg_parser.h"

#include <QDebug>
#include <QDateTime>

#include <cmath>

static constexpr double RAD_2_DEG = 180.0 / M_PI;
static constexpr double DELTA_VEL_LSB = 1.0 / 1048576.0;
static constexpr double DELTA_ANG_LSB = 1.0 / 67108864.0;


SbgErrorCode onEComLogReceived(SbgEComHandle*, SbgEComClass msgClass, SbgEComMsgId msg,
                               const SbgBinaryLogData* pLogData, void* pUserArg)
{
  SbgParser* owner = (SbgParser*)pUserArg;
  if (!owner) {
    return SBG_ERROR;
  }

  if (msgClass != SBG_ECOM_CLASS_LOG_ECOM_0) {
    qDebug("onEComLogReceived() - Not SBG_ECOM_CLASS_LOG_ECOM_0 (%d)", msgClass);
    return SBG_ERROR;
  }

  switch (msg) {
    case SBG_ECOM_LOG_UTC_TIME: owner->onEComLogUtc(pLogData);
      break;
  case SBG_ECOM_LOG_IMU_SHORT: owner->onEComLogShort(pLogData);
    break;
  case SBG_ECOM_LOG_IMU_DATA: owner->onEComLogImu(pLogData);
    break;
  case SBG_ECOM_LOG_EKF_EULER: owner->onEComLogEuler(pLogData);
    break;
  case SBG_ECOM_LOG_EKF_QUAT: owner->onEComLogQuat(pLogData);
    break;
  case SBG_ECOM_LOG_EKF_NAV: owner->onEComLogNav(pLogData);
    break;
  case SBG_ECOM_LOG_GPS1_VEL: owner->onEComLogGpsVel(pLogData);
    break;
  case SBG_ECOM_LOG_GPS1_POS: owner->onEComLogGpsPos(pLogData);
    break;
  case SBG_ECOM_LOG_GPS1_HDT: owner->onEComLogGpsHdt(pLogData);
    break;
  case SBG_ECOM_LOG_AIR_DATA: owner->onEComLogAirData(pLogData);
    break;
  default:
    break;
  }

  return SBG_NO_ERROR;
}

SbgParser::SbgParser()
  : _info()
  , _utcTimestamp(0)
  , _navTimestamp(0)
  , _lastUtcData{}
  , _data()
{
}

SbgParser::~SbgParser()
{
  sbgInterfaceFileClose(&_iface);
}

bool SbgParser::open(const std::string& fileName)
{
  SbgErrorCode errorCode = SBG_NO_ERROR;

  errorCode = sbgInterfaceFileOpen(&_iface, fileName.data());
  if (errorCode != SBG_NO_ERROR) {
    qDebug("SbgParser::open() - sbgInterfaceSerialCreate error: %d", errorCode);
    return false;
  }

  errorCode = sbgEComInit(&_handle, &_iface);
  if (errorCode != SBG_NO_ERROR) {
    qDebug("SbgParser::open() - sbgEComInit error: %d", errorCode);
    return false;
  }

  errorCode = sbgEComSetReceiveLogCallback(&_handle, onEComLogReceived, this);
  if (errorCode != SBG_NO_ERROR) {
    qDebug("SbgParser::open() - sbgEComSetReceiveLogCallback error: %d", errorCode);
    return false;
  }

  _utcTimestamp = 0;
  _navTimestamp = 0;
  _lastUtcData = {};
  clearData();

  errorCode = sbgEComHandle(&_handle);
  if ((errorCode != SBG_NO_ERROR) && (errorCode != SBG_NOT_READY)) {
    qDebug("SbgParser::loop_() - sbgEComHandle error: %d", errorCode);
    return false;
  }

  QDateTime dt = QDateTime(QDate(_lastUtcData.year, _lastUtcData.month, _lastUtcData.day),
                           QTime(_lastUtcData.hour, _lastUtcData.minute, _lastUtcData.second,
                                 _lastUtcData.nanoSecond/1e6), Qt::UTC);
  addInfo("end", dt.toString("yyyyMMdd_hhmmss").toStdString());

  addInfo("session", "full");

  return true;
}

void SbgParser::close()
{
  sbgInterfaceFileClose(&_iface);
}

void SbgParser::onEComLogUtc(const SbgBinaryLogData* pLogData)
{
  const SbgLogUtcData& utcData = pLogData->utcData;

  bool utcValid = (sbgEComLogUtcGetClockUtcStatus(utcData.status) == SBG_ECOM_UTC_VALID);

  if (!_utcTimestamp && _navTimestamp) {
    _utcTimestamp = utcData.timeStamp;
    QDateTime dt = QDateTime(QDate(utcData.year, utcData.month, utcData.day),
                             QTime(utcData.hour, utcData.minute, utcData.second,
                                   utcData.nanoSecond/1e6), Qt::UTC);
    addInfo("begin", dt.toString("yyyyMMdd_hhmmss").toStdString());
  }
  else if (_utcTimestamp && !utcValid) {
    addInfo("session", "reset");
    QDateTime dt = QDateTime(QDate(utcData.year, utcData.month, utcData.day),
                             QTime(utcData.hour, utcData.minute, utcData.second,
                                   utcData.nanoSecond/1e6), Qt::UTC);
    addInfo("end", dt.toString("yyyyMMdd_hhmmss").toStdString());
    _navTimestamp = 0;
  }
  else if (_utcTimestamp && (utcData.timeStamp < _lastUtcData.timeStamp)) {
    addInfo("session", "back");
    QDateTime dt = QDateTime(QDate(utcData.year, utcData.month, utcData.day),
                             QTime(utcData.hour, utcData.minute, utcData.second,
                                   utcData.nanoSecond/1e6), Qt::UTC);
    addInfo("end", dt.toString("yyyyMMdd_hhmmss").toStdString());
    _navTimestamp = 0;
  }
  else if (_utcTimestamp && (utcData.timeStamp > (_lastUtcData.timeStamp+1.2e6))) {
    addInfo("session", "jump");
    QDateTime dt = QDateTime(QDate(utcData.year, utcData.month, utcData.day),
                             QTime(utcData.hour, utcData.minute, utcData.second,
                                   utcData.nanoSecond/1e6), Qt::UTC);
    addInfo("end", dt.toString("yyyyMMdd_hhmmss").toStdString());
    _navTimestamp = 0;
  }
  _lastUtcData = utcData;
}

void SbgParser::onEComLogShort(const SbgBinaryLogData* pLogData)
{
  const SbgLogImuShort& imuShort = pLogData->imuShort;

  if (!_utcTimestamp && !_navTimestamp) return;

  if (_utcTimestamp && !_navTimestamp) return;

  _data[Dvx].push_back({imuShort.timeStamp-_utcTimestamp, imuShort.deltaVelocity[0]*DELTA_VEL_LSB});
  _data[Dvy].push_back({imuShort.timeStamp-_utcTimestamp, imuShort.deltaVelocity[1]*DELTA_VEL_LSB});
  _data[Dvz].push_back({imuShort.timeStamp-_utcTimestamp, imuShort.deltaVelocity[2]*DELTA_VEL_LSB});
  _data[Dax].push_back({imuShort.timeStamp-_utcTimestamp, imuShort.deltaAngle[0]*DELTA_ANG_LSB* RAD_2_DEG});
  _data[Day].push_back({imuShort.timeStamp-_utcTimestamp, imuShort.deltaAngle[1]*DELTA_ANG_LSB*RAD_2_DEG});
  _data[Daz].push_back({imuShort.timeStamp-_utcTimestamp, imuShort.deltaAngle[2]*DELTA_ANG_LSB*RAD_2_DEG});
}

void SbgParser::onEComLogImu(const SbgBinaryLogData* pLogData)
{
  Q_UNUSED(pLogData)
}

void SbgParser::onEComLogEuler(const SbgBinaryLogData* pLogData)
{
  const SbgLogEkfEulerData& ekfEuler = pLogData->ekfEulerData;

  if (!_utcTimestamp && !_navTimestamp) return;

  if (_utcTimestamp && !_navTimestamp) return;

  _data[Roll].push_back({ekfEuler.timeStamp-_utcTimestamp, ekfEuler.euler[0]*RAD_2_DEG});
  _data[Pitch].push_back({ekfEuler.timeStamp-_utcTimestamp, ekfEuler.euler[1]*RAD_2_DEG});
  _data[Yaw].push_back({ekfEuler.timeStamp-_utcTimestamp, ekfEuler.euler[2]*RAD_2_DEG});
}

void SbgParser::onEComLogQuat(const SbgBinaryLogData* pLogData)
{
  Q_UNUSED(pLogData)
}

void SbgParser::onEComLogNav(const SbgBinaryLogData* pLogData)
{
  const SbgLogEkfNavData& ekfNav = pLogData->ekfNavData;

  if (_utcTimestamp && !_navTimestamp) return;

  bool navValid = (sbgEComLogEkfGetSolutionMode(ekfNav.status) == SBG_ECOM_SOL_MODE_NAV_POSITION);

  if (!_navTimestamp && navValid) {
    _navTimestamp = ekfNav.timeStamp;
  }

  if (!_utcTimestamp && !_navTimestamp) return;

  _data[Lat].push_back({ekfNav.timeStamp-_utcTimestamp, ekfNav.position[0]});
  _data[Lon].push_back({ekfNav.timeStamp-_utcTimestamp, ekfNav.position[1]});
  _data[Alt].push_back({ekfNav.timeStamp-_utcTimestamp, ekfNav.position[2]});
  _data[Vn].push_back({ekfNav.timeStamp-_utcTimestamp, ekfNav.velocity[0]});
  _data[Ve].push_back({ekfNav.timeStamp-_utcTimestamp, ekfNav.velocity[1]});
  _data[Vd].push_back({ekfNav.timeStamp-_utcTimestamp, ekfNav.velocity[2]});
}

void SbgParser::onEComLogGpsPos(const SbgBinaryLogData* pLogData)
{
  const SbgLogGpsPos& gpsPos = pLogData->gpsPosData;

  if (!_utcTimestamp && !_navTimestamp) return;

  if (_utcTimestamp && !_navTimestamp) return;

  _data[GnssLat].push_back({gpsPos.timeStamp-_utcTimestamp, gpsPos.latitude});
  _data[GnssLon].push_back({gpsPos.timeStamp-_utcTimestamp, gpsPos.longitude});
  _data[GnssAlt].push_back({gpsPos.timeStamp-_utcTimestamp, gpsPos.altitude});
}

void SbgParser::onEComLogGpsVel(const SbgBinaryLogData* pLogData)
{
  const SbgLogGpsVel& gpsVel = pLogData->gpsVelData;

  if (!_utcTimestamp && !_navTimestamp) return;

  if (_utcTimestamp && !_navTimestamp) return;

  _data[GnssVn].push_back({gpsVel.timeStamp, gpsVel.velocity[0]});
  _data[GnssVe].push_back({gpsVel.timeStamp, gpsVel.velocity[1]});
  _data[GnssVd].push_back({gpsVel.timeStamp, gpsVel.velocity[2]});
}

void SbgParser::onEComLogGpsHdt(const SbgBinaryLogData* pLogData)
{
  Q_UNUSED(pLogData)
}

void SbgParser::onEComLogAirData(const SbgBinaryLogData* pLogData)
{
  Q_UNUSED(pLogData)
}

void SbgParser::clearData()
{
  std::for_each(_data.begin(), _data.end(), [=](std::vector<data_t>& d){
    d.clear();
  });
}

void SbgParser::addInfo(const std::string& key, const std::string& val)
{
  if (_info.find(key) == _info.end()) {
    _info[key] = val;
  }
}
