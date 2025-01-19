// earthwidget.cpp
#include "earthwidget.h"
#include <QMouseEvent>
#include <QTimer>
#include <QPainter>
#include <QtMath>

EarthWidget::EarthWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , camera(EARTH_RADIUS)
    , isMousePressed(false)
    , isAnimating(true)
    , selectedSatelliteId(-1)
    , rotationAngle(0.0f)
{
    setupSurfaceFormat();
    setFocusPolicy(Qt::StrongFocus);
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

    earthRenderer = new EarthRenderer(EARTH_RADIUS);
    satelliteRenderer = new SatelliteRenderer();
    trajectoryRenderer = new TrajectoryRenderer();
    fpsRenderer = new FPSRenderer();

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, [this]() {
        if (isAnimating) {
            rotationAngle += 1.0f;
            update();
        }
    });
    animationTimer->start(16);
}

EarthWidget::~EarthWidget()
{
    makeCurrent();
    delete earthRenderer;
    delete satelliteRenderer;
    delete trajectoryRenderer;
    delete fpsRenderer;
    doneCurrent();
}

void EarthWidget::setupSurfaceFormat()
{
    // Установка формата OpenGL
    QSurfaceFormat format;
    format.setVersion(3, 3);  // Используем OpenGL 3.3
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4); // Мультисэмплинг
    setFormat(format);
    QSurfaceFormat::setDefaultFormat(format);
}

void EarthWidget::initializeGL()
{
    // Убедитесь, что контекст OpenGL активен
    makeCurrent();

    // Инициализируем базовые функции OpenGL для каждого рендерера
    if (!earthRenderer->init() ||
        !satelliteRenderer->init() ||
        !trajectoryRenderer->init()) {
        qDebug() << "Failed to initialize OpenGL functions for renderers";
        return;
    }

    // Теперь можно инициализировать рендереры
    earthRenderer->initialize();
    satelliteRenderer->initialize();
    trajectoryRenderer->initialize();

    // Настройка параметров рендеринга
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Включаем и настраиваем тест глубины
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);  // Добавлено: стандартная функция теста глубины
    glDepthMask(GL_TRUE);  // Добавлено: разрешаем запись в буфер глубины

    // Включаем отсечение задних граней
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);   // Добавлено: отсекаем задние грани
    glFrontFace(GL_CCW);   // Добавлено: определяем порядок вершин для передней грани

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void EarthWidget::resizeGL(int w, int h)
{
    float aspect = float(w) / float(h ? h : 1);
    projection.setToIdentity();
    projection.perspective(45.0f, aspect, EARTH_RADIUS * 0.1f, EARTH_RADIUS * 100.0f);
}

void EarthWidget::paintGL()
{
    // Очищаем буферы цвета и глубины
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Убеждаемся, что тест глубины включен
    glEnable(GL_DEPTH_TEST);

    model.setToIdentity();
    model.rotate(rotationAngle, 0, 1, 0);

    QMatrix4x4 viewMatrix = camera.getViewMatrix();

    // 3D Rendering
    earthRenderer->render(projection, viewMatrix, model);

    if (selectedSatelliteId != -1) {
        trajectoryRenderer->render(projection, viewMatrix, model);
    }

    satelliteRenderer->render(projection, viewMatrix, model);

    // 2D Overlay
    QPainter painter(this);
    fpsRenderer->render(painter, size());
    painter.end();

    // Update FPS counter
    fpsRenderer->update();
}

void EarthWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isMousePressed = true;
        lastMousePos = event->pos();

        int pickedId = pickSatellite(event->pos());
        if (pickedId != selectedSatelliteId) {
            if (selectedSatelliteId != -1) {
                satellites[selectedSatelliteId].isSelected = false;
            }
            selectedSatelliteId = pickedId;
            if (selectedSatelliteId != -1) {
                satellites[selectedSatelliteId].isSelected = true;
            }
            satelliteRenderer->updateSatellites(satellites);
            update();
        }
    }
}

void EarthWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isMousePressed) {
        QPoint delta = event->pos() - lastMousePos;
        camera.rotate(delta.x() * 0.01f, delta.y() * 0.01f);
        lastMousePos = event->pos();
        update();
    }
}

void EarthWidget::wheelEvent(QWheelEvent *event)
{
    float zoomFactor = event->angleDelta().y() > 0 ? 0.9f : 1.1f;
    camera.zoom(zoomFactor);
    update();
}

void EarthWidget::addSatellite(int id, const QVector3D& position, const QString& info)
{
    Satellite satellite(id, position, info);
    satellites[id] = satellite;
    satelliteRenderer->updateSatellites(satellites);
    update();
}

void EarthWidget::updateSatellitePosition(int id, const QVector3D& newPosition,
                                          const QVector<QVector3D>& trajectory,
                                          const QVector<QVector3D>& futureTrajectory,
                                          float angle)
{
    static QTimer updateTimer;
    static bool timerActive = false;

    auto it = satellites.find(id);
    if (it != satellites.end()) {
        it->position = newPosition;
        it->angle = angle;

        if (id == selectedSatelliteId) {
            // Используем таймер для отложенного обновления траектории
            if (!timerActive) {
                timerActive = true;
                updateTimer.singleShot(100, this, [this, trajectory, futureTrajectory]() {
                    trajectoryRenderer->setTrajectories(trajectory, futureTrajectory);
                    timerActive = false;
                    update();
                });
            }
        }

        satelliteRenderer->updateSatellites(satellites);
    }
    update();
}

bool EarthWidget::toggleEarthAnimation()
{
    isAnimating = !isAnimating;
    return isAnimating;
}

int EarthWidget::pickSatellite(const QPoint& mousePos)
{
    float x = (2.0f * mousePos.x()) / width() - 1.0f;
    float y = 1.0f - (2.0f * mousePos.y()) / height();

    QVector4D rayClip(x, y, -1.0f, 1.0f);
    QVector4D rayEye = projection.inverted() * rayClip;
    rayEye.setZ(-1.0f);
    rayEye.setW(0.0f);

    QVector4D rayWorld4 = camera.getViewMatrix().inverted() * rayEye;
    QVector3D rayWorld(rayWorld4.x(), rayWorld4.y(), rayWorld4.z());
    rayWorld.normalize();

    QVector3D rayOrigin = camera.getPosition();
    float minDistance = std::numeric_limits<float>::max();
    int closestSatelliteId = -1;
    float pickRadius = EARTH_RADIUS * 0.1f;

    for (const auto& satellite : satellites) {
        QVector3D satPos = model * satellite.position;
        QVector3D toSatellite = satPos - rayOrigin;
        float projection = QVector3D::dotProduct(toSatellite, rayWorld);

        if (projection < 0) continue;

        QVector3D projectionPoint = rayOrigin + rayWorld * projection;
        float distance = (satPos - projectionPoint).length();

        if (distance < pickRadius && projection < minDistance) {
            minDistance = projection;
            closestSatelliteId = satellite.id;
        }
    }

    emit satelliteSelected(closestSatelliteId);
    return closestSatelliteId;
}
