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
  : _fileName()
  , _utcStr()
  , _altMin(-1)
  , _altMax(-1)
  , _altDelta(-1)
  , _info()
  , _refts(0)
  , _refutc(0)
  , _initDone(false)
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

  _fileName = fileName;

  errorCode = sbgInterfaceFileOpen(&_iface, _fileName.data());
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

  _refts = 0;
  _refutc = 0;
  _initDone = false;
  _data.clear();

  errorCode = sbgEComHandle(&_handle);
  if ((errorCode != SBG_NO_ERROR) && (errorCode != SBG_NOT_READY)) {
    qDebug("SbgParser::loop_() - sbgEComHandle error: %d", errorCode);
    return false;
  }

  _info.insert({ "filename", _fileName });
  _info.insert({ "utc", _utcStr });
  _info.insert({ "alt.min", std::to_string(_altMin) });
  _info.insert({ "alt.max", std::to_string(_altMax) });
  _info.insert({ "alt.delta", std::to_string(_altDelta) });

  resetUtcTimestamp();

  qDebug("done - %s", _utcStr.c_str());

  return true;
}

void SbgParser::close()
{
  sbgInterfaceFileClose(&_iface);
}

void SbgParser::onEComLogUtc(const SbgBinaryLogData* pLogData)
{
  const SbgLogUtcData& utcData = pLogData->utcData;
  if ((_refts == 0) && (_refutc == 0)
      && (sbgEComLogUtcGetClockStatus(utcData.status) == SBG_ECOM_CLOCK_VALID)
      && (sbgEComLogUtcGetClockUtcStatus(utcData.status) == SBG_ECOM_UTC_VALID)) {
    _refts = utcData.timeStamp;
    _refutc = QDateTime(QDate(utcData.year, utcData.month, utcData.day),
                        QTime(utcData.hour, utcData.minute, utcData.second, utcData.nanoSecond/1e6),
                        Qt::UTC).toMSecsSinceEpoch();
    int64_t dts = (int64_t) (pLogData->utcData.timeStamp - _refts) / 1000;
    _utcStr = QDateTime::fromMSecsSinceEpoch(_refutc - dts, Qt::UTC).toString("yyyyMMdd_hhmmss").toStdString();
  }
  if (utcData.timeStamp < _refts) {
    qDebug("reset - %s", _utcStr.c_str());
    _refts = 0;
    _refutc = 0;
    _initDone = false;
    _data.clear();
  }
}

void SbgParser::onEComLogShort(const SbgBinaryLogData* pLogData)
{
  const SbgLogImuShort& imuShort = pLogData->imuShort;

  if (!_initDone) return;

  if (_data.empty()) return; // first data after init and no IMU data

  _data.back().val[Dvx] = imuShort.deltaVelocity[0]*DELTA_VEL_LSB;
  _data.back().val[Dvy] = imuShort.deltaVelocity[1]*DELTA_VEL_LSB;
  _data.back().val[Dvz] = imuShort.deltaVelocity[2]*DELTA_VEL_LSB;
  _data.back().val[Dax] = imuShort.deltaAngle[0]*DELTA_ANG_LSB* RAD_2_DEG;
  _data.back().val[Day] = imuShort.deltaAngle[1]*DELTA_ANG_LSB*RAD_2_DEG;
  _data.back().val[Daz] = imuShort.deltaAngle[2]*DELTA_ANG_LSB*RAD_2_DEG;
}

void SbgParser::onEComLogImu(const SbgBinaryLogData* pLogData)
{
  Q_UNUSED(pLogData)
}

void SbgParser::onEComLogEuler(const SbgBinaryLogData* pLogData)
{
  const SbgLogEkfEulerData& ekfEuler = pLogData->ekfEulerData;

  if (!_initDone) return;

  _data.push_back({ekfEuler.timeStamp,{}});

  _data.back().val[Roll] = ekfEuler.euler[0]*RAD_2_DEG;
  _data.back().val[Pitch] = ekfEuler.euler[1]*RAD_2_DEG;
  _data.back().val[Yaw] = ekfEuler.euler[2]*RAD_2_DEG;
}

void SbgParser::onEComLogQuat(const SbgBinaryLogData* pLogData)
{
  Q_UNUSED(pLogData)
  if (!_initDone) return;
}

void SbgParser::onEComLogNav(const SbgBinaryLogData* pLogData)
{
  const SbgLogEkfNavData& ekfNav = pLogData->ekfNavData;

  if (!_initDone
      && (sbgEComLogEkfGetSolutionMode(ekfNav.status) != SBG_ECOM_SOL_MODE_NAV_POSITION)) return;

  _initDone = true;

  if (_data.empty()) return; // first data after init and no IMU data

  _data.back().val[Lat] = ekfNav.position[0];
  _data.back().val[Lon] = ekfNav.position[1];
  _data.back().val[Alt] = ekfNav.position[2];
  _data.back().val[Vn] = ekfNav.velocity[0];
  _data.back().val[Ve] = ekfNav.velocity[1];
  _data.back().val[Vd] = ekfNav.velocity[2];

  float alt = ekfNav.position[2];
  if ((_altMin < 0) || (alt < _altMin)) {
    _altMin = alt;
  }
  if ((_altMax < 0) || (alt > _altMax)) {
    _altMax = alt;
  }
  _altDelta = _altMax - _altMin;
}

void SbgParser::onEComLogGpsPos(const SbgBinaryLogData* pLogData)
{
  Q_UNUSED(pLogData)
}

void SbgParser::onEComLogGpsVel(const SbgBinaryLogData* pLogData)
{
  Q_UNUSED(pLogData)
}

void SbgParser::onEComLogGpsHdt(const SbgBinaryLogData* pLogData)
{
  Q_UNUSED(pLogData)
}

void SbgParser::onEComLogAirData(const SbgBinaryLogData* pLogData)
{
  Q_UNUSED(pLogData)
}

void SbgParser::resetUtcTimestamp()
{
  std::for_each(_data.begin(), _data.end(), [=](data_t& d){
    int64_t dts = (int64_t) (d.utc - _refts) / 1000;
    d.utc = _refutc + dts;
  });
}
