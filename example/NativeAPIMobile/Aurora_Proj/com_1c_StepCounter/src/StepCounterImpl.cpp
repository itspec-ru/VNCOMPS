#include "StepCounterImpl.h"
#include <QAccelerometer>
#include <cmath>

AuroraStepCounterImpl::AuroraStepCounterImpl(QObject* parent)
    : QObject(parent),
      m_accelerometer(new QAccelerometer()),
      m_steps(0),
      m_aboveThreshold(false)
{
    if (m_accelerometer)
    {
        m_connections.push_back(
            connect(m_accelerometer, &QAccelerometer::readingChanged,
                    this, &AuroraStepCounterImpl::handleReading));

        m_accelerometer->setActive(true);
    }
}

AuroraStepCounterImpl::~AuroraStepCounterImpl()
{
    for (QMetaObject::Connection& connection : m_connections)
        disconnect(connection);

    if (m_accelerometer)
    {
        m_accelerometer->setActive(false);
        m_accelerometer->deleteLater();
        m_accelerometer = nullptr;
    }
}

void AuroraStepCounterImpl::start()
{
    if (m_accelerometer)
        m_accelerometer->start();

    m_lastStepTime.start();
}

void AuroraStepCounterImpl::stop()
{
    if (m_accelerometer)
        m_accelerometer->stop();
}

QString AuroraStepCounterImpl::getErrorString()
{
    if (!m_accelerometer)
        return QString();

    QString errorString;

    switch (m_accelerometer->error())
    {
    case 0:
        errorString = "NoError";
        break;
    case 1:
        errorString = "SENSORS_NO_ERROR";
        break;
    case 2:
        errorString = "SENSORS_UNSPECIFIED_ERROR";
        break;
    case 3:
        errorString = "SENSORS_NULL_ERROR";
        break;
    case 4:
        errorString = "SENSORS_ENGINE_ERROR";
        break;
    case 5:
        errorString = "SENSORS_BLOCKED_ERROR";
        break;
    }

    return errorString;
}

AccelerometerData AuroraStepCounterImpl::getAccelerometerData() const
{
    AccelerometerData data;

    if (m_accelerometer)
    {
        if (QAccelerometerReading* const reading =  m_accelerometer->reading())
        {
            data.x = reading->x();
            data.y = reading->y();
            data.z = reading->z();
        }
    }

    return data;
}

void AuroraStepCounterImpl::handleReading()
{
    if (!m_accelerometer)
        return;

    QAccelerometerReading* const reading = m_accelerometer->reading();
    if (!reading)
        return;

    const double x = reading->x();
    const double y = reading->y();
    const double z = reading->z();
    const double magnitude = std::sqrt(x*x + y*y + z*z);

    if (magnitude > m_threshold)
    {
        if (!m_aboveThreshold)
        {
            if (m_lastStepTime.elapsed() > m_stepDelayMs)
            {
                ++m_steps;
                m_aboveThreshold = true;
                m_lastStepTime.restart();

                emit stepCountChanged();
            }
        }
    }
    else if (magnitude < (m_threshold - 1.0))
        m_aboveThreshold = false;
}
