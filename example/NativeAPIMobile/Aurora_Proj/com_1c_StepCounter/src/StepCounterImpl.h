#ifndef STEPCOUNTERIMPL_H
#define STEPCOUNTERIMPL_H

#include <QObject>
#include <QTime>

class QAccelerometer;

struct AccelerometerData
{
    double x {};
    double y {};
    double z {};
};

class AuroraStepCounterImpl : public QObject
{
    Q_OBJECT
public:
    explicit AuroraStepCounterImpl(QObject* parent = nullptr);
    ~AuroraStepCounterImpl();

    int getSteps() const { return m_steps; }
    QString getErrorString();
    AccelerometerData getAccelerometerData() const;

signals:
    void stepCountChanged();

public slots:
    void start();
    void stop();

private slots:
    void handleReading();

private:
    const double    m_threshold      {11.5};
    const double    m_stepDelayMs    {250.0};
    QAccelerometer* m_accelerometer  {};
    int             m_steps          {};
    bool            m_aboveThreshold {};
    QTime           m_lastStepTime;

    std::vector<QMetaObject::Connection> m_connections;
};

#endif // STEPCOUNTERIMPL_H
