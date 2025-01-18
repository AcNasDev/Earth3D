#include <QApplication>
#include <QMainWindow>
#include <QTimer>
#include <QDateTime>
#include <cmath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include "earthwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMainWindow mainWindow;

    // Создаем центральный виджет и layout
    QWidget* centralWidget = new QWidget(&mainWindow);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    // Создаем горизонтальный layout для EarthWidget и информационной панели
    QHBoxLayout* mainLayout = new QHBoxLayout();
    layout->addLayout(mainLayout);

    // Создаем EarthWidget
    EarthWidget* earthWidget = new EarthWidget(centralWidget);
    mainLayout->addWidget(earthWidget, 4); // Соотношение 4:1

    // Создаем панель информации
    QWidget* infoPanel = new QWidget(centralWidget);
    QVBoxLayout* infoPanelLayout = new QVBoxLayout(infoPanel);
    QLabel* infoLabel = new QLabel("Satellite Information:", infoPanel);
    QLabel* satelliteInfo = new QLabel("No satellite selected", infoPanel);
    satelliteInfo->setWordWrap(true);
    infoPanelLayout->addWidget(infoLabel);
    infoPanelLayout->addWidget(satelliteInfo);
    infoPanelLayout->addStretch();
    mainLayout->addWidget(infoPanel, 1); // Соотношение 4:1

    // Создаем кнопку для управления вращением Земли
    QPushButton* earthRotationButton = new QPushButton("Stop Earth Rotation", centralWidget);
    layout->addWidget(earthRotationButton);

    // Устанавливаем центральный виджет
    mainWindow.setCentralWidget(centralWidget);

    // Обработчик нажатия кнопки вращения Земли
    QObject::connect(earthRotationButton, &QPushButton::clicked, [earthWidget, earthRotationButton]() {
        bool isAnimating = earthWidget->toggleEarthAnimation();
        earthRotationButton->setText(isAnimating ? "Stop Earth Rotation" : "Start Earth Rotation");
    });

    // Создаем спутники с разными скоростями вращения
    const float EARTH_RADIUS = 6371000.0f; // Радиус Земли в метрах
    const float ORBIT_RADIUS = EARTH_RADIUS * 1.5f;

    struct SatelliteData {
        float angle;
        float speed;
        int id;
        QVector3D position;

        // Функция расчета траекторий для дуги в 30 градусов
        void calculateTrajectories(float orbitRadius, QVector<QVector3D>& trajectory, QVector<QVector3D>& futureTrajectory) {
            trajectory.clear();

            // Расчет траектории: 30 градусов назад и вперед от текущей позиции
            const int arcPoints = 30; // количество точек для дуги в 30 градусов
            const float arcRange = 30.0f; // диапазон в градусах

            // Рассчитываем точки для текущей дуги траектории
            for (int i = -arcPoints; i <= arcPoints; ++i) {
                float arcAngle = qDegreesToRadians(angle + (i * arcRange / arcPoints));
                trajectory.append(QVector3D(
                    orbitRadius * cos(arcAngle),
                    0.0f,
                    orbitRadius * sin(arcAngle)
                    ));
            }

            // Расчет будущей траектории (следующие 30 градусов)
            futureTrajectory.clear();
            const int futurePoints = 30;
            const float predictionTime = 5.0f; // время прогноза в секундах
            float timeStep = predictionTime / futurePoints;

            for (int i = 0; i <= futurePoints; ++i) {
                float futureTime = timeStep * i;
                float futureAngle = qDegreesToRadians(angle + speed * futureTime);
                futureTrajectory.append(QVector3D(
                    orbitRadius * cos(futureAngle),
                    0.0f,
                    orbitRadius * sin(futureAngle)
                    ));
            }
        }
    };

    QVector<SatelliteData> satelliteData;

    satelliteData.append({0.0f, 1.0f, 1});
    satelliteData.append({72.0f, 2.0f, 2});
    satelliteData.append({144.0f, 3.0f, 3});
    satelliteData.append({216.0f, 4.0f, 4});
    satelliteData.append({288.0f, 5.0f, 5});

    // Обновление информации о выбранном спутнике
    QObject::connect(earthWidget, &EarthWidget::satelliteSelected,
                     [satelliteInfo, &satelliteData, ORBIT_RADIUS](int id) {
                         if (id == -1) {
                             satelliteInfo->setText("No satellite selected");
                             return;
                         }

                         for (const auto& sat : satelliteData) {
                             if (sat.id == id) {
                                 QString info = QString(
                                                    "Satellite ID: %1\n"
                                                    "Speed: %2°/s\n"
                                                    "Current Angle: %3°\n"
                                                    "Position:\n"
                                                    "X: %4 m\n"
                                                    "Y: %5 m\n"
                                                    "Z: %6 m\n"
                                                    "Orbit Radius: %7 km\n"
                                                    "Time: %8"
                                                    )
                                                    .arg(sat.id)
                                                    .arg(sat.speed)
                                                    .arg(sat.angle, 0, 'f', 2)
                                                    .arg(sat.position.x(), 0, 'f', 2)
                                                    .arg(sat.position.y(), 0, 'f', 2)
                                                    .arg(sat.position.z(), 0, 'f', 2)
                                                    .arg(ORBIT_RADIUS / 1000.0, 0, 'f', 2)
                                                    .arg(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss"));

                                 satelliteInfo->setText(info);
                                 break;
                             }
                         }
                     });

    // Таймер для обновления позиций спутников
    QTimer* timer = new QTimer(&mainWindow);
    // В таймере обновления позиций:
    QObject::connect(timer, &QTimer::timeout, [=, &satelliteData]() mutable {
        for(auto& sat : satelliteData) {
            sat.angle += sat.speed * (16.0f / 1000.0f);
            if(sat.angle >= 360.0f) {
                sat.angle -= 360.0f;
            }

            float radians = qDegreesToRadians(sat.angle);
            sat.position = QVector3D(
                ORBIT_RADIUS * cos(radians),
                0.0f,
                ORBIT_RADIUS * sin(radians)
                );

            // Обновляем траектории
            QVector<QVector3D> trajectory, futureTrajectory;
            sat.calculateTrajectories(ORBIT_RADIUS, trajectory, futureTrajectory);

            earthWidget->updateSatellitePosition(
                sat.id,
                sat.position,
                trajectory,
                futureTrajectory,
                sat.angle
                );

            if (earthWidget->getSelectedSatelliteId() == sat.id) {
                emit earthWidget->satelliteSelected(sat.id);
            }
        }
    });

    // При инициализации спутников:
    for(auto& sat : satelliteData) {
        float radians = qDegreesToRadians(sat.angle);
        sat.position = QVector3D(
            ORBIT_RADIUS * cos(radians),
            0.0f,
            ORBIT_RADIUS * sin(radians)
            );

        QVector<QVector3D> trajectory, futureTrajectory;
        sat.calculateTrajectories(ORBIT_RADIUS, trajectory, futureTrajectory);

        earthWidget->addSatellite(
            sat.id,
            sat.position,
            QString("Satellite %1 (Speed: %2°/s)").arg(sat.id).arg(sat.speed),
            trajectory,
            futureTrajectory,
            sat.angle,
            sat.speed
            );
    }

    // Запускаем таймер
    timer->start(16);

    mainWindow.resize(1024, 768);
    mainWindow.show();

    return a.exec();
}
