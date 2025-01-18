#include "earthwidget.h"
#include <QMouseEvent>
#include <QTimer>
#include <cmath>
#include <QPainter>
#include <QDateTime>
#include <QImageReader>

EarthWidget::EarthWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , earthTexture(nullptr)
    , cameraTheta(0.0f)
    , cameraPhi(M_PI_2)
    , cameraZoom(EARTH_RADIUS * 3.0f)  // Устанавливаем начальное расстояние камеры
    , isMousePressed(false)
    , isAnimating(true)
    , selectedSatelliteId(-1)
    , rotationAngle(0.0f)
{
    setFocusPolicy(Qt::StrongFocus);

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
    delete earthTexture;
    delete heightMapTexture;
    delete normalMapTexture;
    sphereVBO.destroy();
    sphereVAO.destroy();
    doneCurrent();
}

void EarthWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Настройка буфера глубины
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);  // Отсекаем задние грани

    initShaders();
    initTextures();
    initSphereGeometry();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void EarthWidget::initShaders()
{
    // Earth shader
    earthProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/earth_vertex.glsl");
    earthProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/earth_fragment.glsl");
    earthProgram.link();

    // Satellite shader
    satelliteProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/sat_vertex.glsl");
    satelliteProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/sat_fragment.glsl");
    satelliteProgram.link();

    // Line shader
    lineProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/line_vertex.glsl");
    lineProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/line_fragment.glsl");
    lineProgram.link();
}

void EarthWidget::initTextures()
{
    QImageReader::setAllocationLimit(0);

    QString buildDir = QCoreApplication::applicationDirPath();

    // Загружаем текстуры из папки сборки
    QImage earthImage(buildDir + "/textures/earth.jpg");
    if (earthImage.isNull()) {
        qDebug() << "Failed to load earth texture";
        return;
    }
    earthTexture = new QOpenGLTexture(earthImage.mirrored());
    earthTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    earthTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    earthTexture->setWrapMode(QOpenGLTexture::Repeat);

    QImage heightImage(buildDir + "/textures/earth_height.png");
    if (heightImage.isNull()) {
        qDebug() << "Failed to load height map";
        return;
    }
    heightMapTexture = new QOpenGLTexture(heightImage.mirrored());
    heightMapTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    heightMapTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    heightMapTexture->setWrapMode(QOpenGLTexture::Repeat);

    QImage normalImage(buildDir + "/textures/earth_normal.png");
    if (normalImage.isNull()) {
        qDebug() << "Failed to load normal map";
        return;
    }
    normalMapTexture = new QOpenGLTexture(normalImage.mirrored());
    normalMapTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    normalMapTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    normalMapTexture->setWrapMode(QOpenGLTexture::Repeat);
}

void EarthWidget::initSphereGeometry()
{
    const int segments = 1024;
    const int rings = 512;
    QVector<GLfloat> vertices;

    // Генерация вершин сферы
    for (int ring = 0; ring <= rings; ++ring) {
        float phi = ring * M_PI / rings;
        for (int segment = 0; segment <= segments; ++segment) {
            float theta = segment * 2.0f * M_PI / segments;

            // Позиция
            float x = sin(phi) * cos(theta);
            float y = cos(phi);
            float z = sin(phi) * sin(theta);

            // Добавляем позицию
            vertices << x << y << z;

            // Текстурные координаты - инвертируем t координату (1.0 - t)
            vertices << (1.0f - static_cast<float>(segment) / segments)
                     << (1.0f - static_cast<float>(ring) / rings);  // Изменено здесь

            // Нормали
            vertices << x << y << z;
        }
    }

    // Генерация индексов
    QVector<GLuint> indices;
    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            GLuint first = ring * (segments + 1) + segment;
            GLuint second = first + segments + 1;

            indices << first << first + 1 << second;
            indices << second << first + 1 << second + 1;
        }
    }

    sphereVertexCount = indices.size();

    // Создаем и настраиваем VAO
    sphereVAO.create();
    sphereVAO.bind();

    // Создаем и заполняем VBO для вершин
    sphereVBO.create();
    sphereVBO.bind();
    sphereVBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
    sphereVBO.allocate(vertices.constData(), vertices.size() * sizeof(GLfloat));

    // Создаем и заполняем индексный буфер
    QOpenGLBuffer indexBuffer(QOpenGLBuffer::IndexBuffer);
    indexBuffer.create();
    indexBuffer.bind();
    indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    indexBuffer.allocate(indices.constData(), indices.size() * sizeof(GLuint));

    // Настраиваем атрибуты вершин
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          reinterpret_cast<void*>(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // Освобождаем буферы
    sphereVAO.release();
    sphereVBO.release();
    indexBuffer.release();
}

void EarthWidget::resizeGL(int w, int h)
{
    float aspect = float(w) / float(h ? h : 1);
    projection.setToIdentity();
    projection.perspective(45.0f, aspect, EARTH_RADIUS * 0.1f, EARTH_RADIUS * 100.0f);
}

void EarthWidget::paintGL()
{
    // Очищаем буфер цвета и глубины
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Включаем тест глубины
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    // Обновляем позицию камеры
    cameraPosition = QVector3D(
        cameraZoom * sin(cameraPhi) * cos(cameraTheta),
        cameraZoom * cos(cameraPhi),
        cameraZoom * sin(cameraPhi) * sin(cameraTheta)
        );

    view.setToIdentity();
    view.lookAt(cameraPosition, QVector3D(0, 0, 0), QVector3D(0, 1, 0));

    model.setToIdentity();
    model.rotate(rotationAngle, 0, 1, 0);

    // Рисуем объекты с учетом глубины
    drawEarth();
    drawSatellites();

    // Отключаем тест глубины для 2D элементов
    glDisable(GL_DEPTH_TEST);

    if (selectedSatelliteId != -1) {
        drawTrajectories();
    }
    drawSatellitesInfo();
}

QMatrix4x4 EarthWidget::getMVPMatrix() const
{
    QMatrix4x4 mvp = projection * view * model;
    return mvp;
}

void EarthWidget::drawSatellitesInfo()
{
    glDisable(GL_DEPTH_TEST);
    QPainter painter(this);
    painter.beginNativePainting();

    for (const auto& satellite : satellites) {
        if (satellite.isSelected) {
            // Получаем экранные координаты для позиции спутника
            QVector4D clipSpace = getMVPMatrix() * QVector4D(satellite.position, 1.0f);
            QVector3D ndc = QVector3D(clipSpace.x(), clipSpace.y(), clipSpace.z()) / clipSpace.w();
            QPoint screenPos(
                (ndc.x() + 1.0f) * width() / 2.0f,
                (1.0f - ndc.y()) * height() / 2.0f
                );

            painter.end();
            painter.begin(this);

            drawSatelliteInfo(painter, satellite, screenPos);

            painter.end();
            painter.begin(this);
            painter.beginNativePainting();
            break;
        }
    }

    painter.end();
}

void EarthWidget::drawSatelliteInfo(QPainter& painter, const Satellite& satellite, const QPoint& screenPos)
{
    // Настраиваем шрифт
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    QFontMetrics fm(font);

    // Форматируем информацию о спутнике
    QString info = QString(
                       "ID: %1\n"
                       "Speed: %2°/s\n"
                       "Pos: X:%3.2f Y:%4.2f Z:%5.2f"
                       )
                       .arg(satellite.id)
                       .arg(satellite.info.split("Speed: ")[1].split("°")[0])
                       .arg(satellite.position.x())
                       .arg(satellite.position.y())
                       .arg(satellite.position.z());

    // Вычисляем размеры текстового блока
    QStringList lines = info.split('\n');
    int maxWidth = 0;
    int totalHeight = 0;
    for (const QString& line : lines) {
        maxWidth = qMax(maxWidth, fm.horizontalAdvance(line));
        totalHeight += fm.height();
    }

    // Рисуем полупрозрачный фон
    QRect bgRect(
        screenPos.x(),
        screenPos.y() - totalHeight/2,
        maxWidth + 20,
        totalHeight + 10
        );
    painter.fillRect(bgRect, QColor(0, 0, 0, 128));

    // Рисуем текст
    painter.setPen(Qt::white);
    int y = screenPos.y() - totalHeight/2 + 5;
    for (const QString& line : lines) {
        painter.drawText(screenPos.x() + 10, y + fm.height(), line);
        y += fm.height();
    }
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
            update();
        }
    }
}

void EarthWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isMousePressed) {
        QPoint delta = event->pos() - lastMousePos;
        cameraTheta -= delta.x() * 0.01f;
        cameraPhi = qBound(0.1f, cameraPhi - delta.y() * 0.01f, float(M_PI - 0.1f));
        lastMousePos = event->pos();
        update();
    }
}

void EarthWidget::wheelEvent(QWheelEvent *event)
{
    float zoomFactor = event->angleDelta().y() > 0 ? 0.9f : 1.1f;
    cameraZoom = qBound(EARTH_RADIUS * 1.5f, cameraZoom * zoomFactor, EARTH_RADIUS * 10.0f);
    update();
}

void EarthWidget::drawEarth()
{
    earthProgram.bind();

    QMatrix4x4 earthMatrix = model;
    earthMatrix.scale(EARTH_RADIUS);

    earthProgram.setUniformValue("mvp", projection * view * earthMatrix);
    earthProgram.setUniformValue("model", earthMatrix);
    earthProgram.setUniformValue("viewPos", cameraPosition);

    // Устанавливаем масштаб смещения (можно настраивать)
    earthProgram.setUniformValue("displacementScale", 0.05f);

    // Привязываем текстуры
    earthTexture->bind(0);
    heightMapTexture->bind(1);
    normalMapTexture->bind(2);

    earthProgram.setUniformValue("earthTexture", 0);
    earthProgram.setUniformValue("heightMap", 1);
    earthProgram.setUniformValue("normalMap", 2);

    sphereVAO.bind();
    glDrawElements(GL_TRIANGLES, sphereVertexCount, GL_UNSIGNED_INT, 0);
    sphereVAO.release();

    earthProgram.release();
}

void EarthWidget::drawSatellites()
{
    // Включаем тест глубины
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    satelliteProgram.bind();
    sphereVAO.bind();

    for (const auto& satellite : satellites) {
        QMatrix4x4 satMatrix = model;
        satMatrix.translate(satellite.position);

        QVector3D satelliteWorldPos = model * satellite.position;
        QVector3D toCameraVector = cameraPosition - satelliteWorldPos;
        float distanceToCamera = toCameraVector.length();

        float scale = distanceToCamera * 0.005f;
        satMatrix.scale(scale);

        satelliteProgram.setUniformValue("mvp", projection * view * satMatrix);
        satelliteProgram.setUniformValue("isSelected", satellite.isSelected);

        glDrawElements(GL_TRIANGLES, sphereVertexCount, GL_UNSIGNED_INT, 0);
    }

    sphereVAO.release();
    satelliteProgram.release();
}

void EarthWidget::drawTrajectories()
{
    if (selectedSatelliteId == -1) return;

    const auto& satellite = satellites[selectedSatelliteId];

    lineProgram.bind();
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Отрисовка полной орбиты (белая линия)
    if (!satellite.trajectory.isEmpty()) {
        QOpenGLBuffer trajectoryVBO(QOpenGLBuffer::VertexBuffer);
        trajectoryVBO.create();
        trajectoryVBO.bind();
        trajectoryVBO.allocate(satellite.trajectory.constData(),
                               satellite.trajectory.size() * sizeof(QVector3D));

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        lineProgram.setUniformValue("mvp", projection * view * model);
        lineProgram.setUniformValue("color", QVector4D(1.0f, 1.0f, 1.0f, 0.3f));

        glLineWidth(1.0f);
        // Используем GL_LINE_STRIP вместо GL_LINE_LOOP
        glDrawArrays(GL_LINE_STRIP, 0, satellite.trajectory.size());

        trajectoryVBO.release();
        trajectoryVBO.destroy();
    }

    // Отрисовка будущей траектории (голубая линия)
    if (!satellite.futureTrajectory.isEmpty()) {
        QOpenGLBuffer futureVBO(QOpenGLBuffer::VertexBuffer);
        futureVBO.create();
        futureVBO.bind();
        futureVBO.allocate(satellite.futureTrajectory.constData(),
                           satellite.futureTrajectory.size() * sizeof(QVector3D));

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        lineProgram.setUniformValue("mvp", projection * view * model);
        lineProgram.setUniformValue("color", QVector4D(0.2f, 0.6f, 1.0f, 1.0f)); // яркий голубой

        glLineWidth(2.0f);
        glDrawArrays(GL_LINE_STRIP, 0, satellite.futureTrajectory.size());

        futureVBO.release();
        futureVBO.destroy();
    }

    lineProgram.release();
    glDisable(GL_LINE_SMOOTH);
}

int EarthWidget::pickSatellite(const QPoint& mousePos)
{
    // Преобразуем координаты мыши в нормализованные координаты устройства (-1 до 1)
    float x = (2.0f * mousePos.x()) / width() - 1.0f;
    float y = 1.0f - (2.0f * mousePos.y()) / height();

    // Создаем луч в пространстве камеры
    QVector4D rayClip(x, y, -1.0f, 1.0f);

    // Преобразуем в пространство глаза
    QVector4D rayEye = projection.inverted() * rayClip;
    rayEye.setZ(-1.0f);
    rayEye.setW(0.0f);

    // Преобразуем в мировое пространство
    QVector4D rayWorld4 = view.inverted() * rayEye;
    QVector3D rayWorld(rayWorld4.x(), rayWorld4.y(), rayWorld4.z());
    rayWorld.normalize();

    // Начальная позиция луча (позиция камеры)
    QVector3D rayOrigin = cameraPosition;

    // Находим ближайший спутник
    float minDistance = std::numeric_limits<float>::max();
    int closestSatelliteId = -1;
    float pickRadius = EARTH_RADIUS * 0.1f; // Радиус для определения попадания

    for (const auto& satellite : satellites) {
        // Получаем позицию спутника в мировых координатах
        QVector3D satPos = model * satellite.position;

        // Вектор от начала луча до спутника
        QVector3D toSatellite = satPos - rayOrigin;

        // Проекция вектора toSatellite на направление луча
        float projection = QVector3D::dotProduct(toSatellite, rayWorld);

        // Если спутник находится позади камеры, пропускаем его
        if (projection < 0) continue;

        // Находим ближайшую точку на луче к спутнику
        QVector3D projectionPoint = rayOrigin + rayWorld * projection;

        // Расстояние от спутника до луча
        float distance = (satPos - projectionPoint).length();

        // Если расстояние меньше порога и это ближайший спутник
        if (distance < pickRadius && projection < minDistance) {
            minDistance = projection;
            closestSatelliteId = satellite.id;
        }
    }

    emit satelliteSelected(closestSatelliteId);
    return closestSatelliteId;
}

void EarthWidget::addSatellite(int id, const QVector3D& position, const QString& info,
                               const QVector<QVector3D>& trajectory,
                               const QVector<QVector3D>& futureTrajectory,
                               float angle, float speed)
{
    Satellite satellite(id, position, info, angle, speed);

    // Копируем траектории
    satellite.trajectory = trajectory;
    satellite.futureTrajectory = futureTrajectory;

    // Добавляем спутник в коллекцию
    satellites[id] = satellite;
    update();
}

void EarthWidget::updateSatellitePosition(int id, const QVector3D& newPosition,
                                          const QVector<QVector3D>& trajectory,
                                          const QVector<QVector3D>& futureTrajectory,
                                          float angle)
{
    auto it = satellites.find(id);
    if (it != satellites.end()) {
        it->position = newPosition;
        it->angle = angle;
        it->trajectory = trajectory;
        it->futureTrajectory = futureTrajectory;
    }
    update(); // Запрашиваем перерисовку виджета
}

bool EarthWidget::toggleEarthAnimation()
{
    isAnimating = !isAnimating;
    return isAnimating;
}

int EarthWidget::getSelectedSatelliteId() const
{
    for (const auto& satellite : satellites) {
        if (satellite.isSelected) {
            return satellite.id;
        }
    }
    return -1;
}
