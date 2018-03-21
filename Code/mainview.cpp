#include "mainview.h"
#include "model.h"
#include "vertex.h"
#include "disk.h"

#include <math.h>
#include <QDateTime>

/**
 * @brief MainView::MainView
 *
 * Constructor of MainView
 *
 * @param parent
 */
MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {
    qDebug() << "MainView constructor";

    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
}

/**
 * @brief MainView::~MainView
 *
 * Destructor of MainView
 * This is the last function called, before exit of the program
 * Use this to clean up your variables, buffers etc.
 *
 */
MainView::~MainView() {
    debugLogger->stopLogging();

    qDebug() << "MainView destructor";

    glDeleteTextures(1, &blueTexturePtr);
    glDeleteTextures(1, &yellowTexturePtr);
    glDeleteTextures(1, &redTexturePtr);
    //glDeleteTextures(1, &object4texturePtr);

    destroyModelBuffers();
}

// --- OpenGL initialization

/**
 * @brief MainView::initializeGL
 *
 * Called upon OpenGL initialization
 * Attaches a debugger and calls other init functions
 */
void MainView::initializeGL() {
    qDebug() << ":: Initializing OpenGL";
    initializeOpenGLFunctions();

    debugLogger = new QOpenGLDebugLogger();
    connect( debugLogger, SIGNAL( messageLogged( QOpenGLDebugMessage ) ),
             this, SLOT( onMessageLogged( QOpenGLDebugMessage ) ), Qt::DirectConnection );

    if ( debugLogger->initialize() ) {
        qDebug() << ":: Logging initialized";
        debugLogger->startLogging( QOpenGLDebugLogger::SynchronousLogging );
        debugLogger->enableMessages();
    }

    QString glVersion;
    glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0, 1.0, 0.0, 1.0);

    createShaderProgram();
    loadMesh();
    loadTextures();

    // Initialize transformations
    updateProjectionTransform();
    updateModelTransforms();

    timer.start(1000.0 / 60.0);
    //qDebug() << "Interval: " << timer.interval();
    //qDebug() << "Time elapsed: " << QTime.elapsed();
}

void MainView::createShaderProgram()
{
    // Create Normal Shader program
    normalShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/shaders/vertshader_normal.glsl");
    normalShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/shaders/fragshader_normal.glsl");
    normalShaderProgram.link();

    // Create Gouraud Shader program
    gouraudShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/shaders/vertshader_gouraud.glsl");
    gouraudShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/shaders/fragshader_gouraud.glsl");
    gouraudShaderProgram.link();

    // Create Phong Shader program
    phongShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/shaders/vertshader_phong.glsl");
    phongShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/shaders/fragshader_phong.glsl");
    phongShaderProgram.link();

    // Get the uniforms for the normal shader.
    uniformModelViewTransformNormal  = normalShaderProgram.uniformLocation("modelViewTransform");
    uniformProjectionTransformNormal = normalShaderProgram.uniformLocation("projectionTransform");
    uniformNormalTransformNormal     = normalShaderProgram.uniformLocation("normalTransform");

    // Get the uniforms for the gouraud shader.
    uniformModelViewTransformGouraud  = gouraudShaderProgram.uniformLocation("modelViewTransform");
    uniformProjectionTransformGouraud = gouraudShaderProgram.uniformLocation("projectionTransform");
    uniformNormalTransformGouraud     = gouraudShaderProgram.uniformLocation("normalTransform");
    uniformMaterialGouraud            = gouraudShaderProgram.uniformLocation("material");
    uniformLightPositionGouraud       = gouraudShaderProgram.uniformLocation("lightPosition");
    uniformLightColourGouraud         = gouraudShaderProgram.uniformLocation("lightColour");
    uniformTexture1SamplerGouraud     = gouraudShaderProgram.uniformLocation("texture1Sampler");

    // Get the uniforms for the phong shader.
    uniformModelViewTransformPhong  = phongShaderProgram.uniformLocation("modelViewTransform");
    uniformProjectionTransformPhong = phongShaderProgram.uniformLocation("projectionTransform");
    uniformNormalTransformPhong     = phongShaderProgram.uniformLocation("normalTransform");
    uniformMaterialPhong            = phongShaderProgram.uniformLocation("material");
    uniformLightPositionPhong       = phongShaderProgram.uniformLocation("lightPosition");
    uniformLightColourPhong         = phongShaderProgram.uniformLocation("lightColour");
    uniformTexture1SamplerPhong     = phongShaderProgram.uniformLocation("texture1Sampler");
}

void MainView::loadMesh()
{
    // Board Model
    Model model1(":/models/connect4text.obj");
    model1.unitize();
    QVector<float> boardData = model1.getVNTInterleaved();

    this->boardSize = model1.getVertices().size();

    // Generate VAO
    glGenVertexArrays(1, &boardVAO);
    glBindVertexArray(boardVAO);

    // Generate VBO
    glGenBuffers(1, &boardVBO);
    glBindBuffer(GL_ARRAY_BUFFER, boardVBO);

    // Write the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, boardData.size() * sizeof(float), boardData.data(), GL_STATIC_DRAW);

    // Set vertex coordinates to location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Set vertex normals to location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set vertex texture coordinates to location 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Disk Model
    Model model2(":/models/disktext.obj");
    model2.unitize();
    QVector<float> diskData = model2.getVNTInterleaved();

    this->diskSize = model2.getVertices().size();

    // Generate VAO
    glGenVertexArrays(1, &diskVAO);
    glBindVertexArray(diskVAO);

    // Generate VBO
    glGenBuffers(1, &diskVBO);
    glBindBuffer(GL_ARRAY_BUFFER, diskVBO);

    // Write the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, diskData.size() * sizeof(float), diskData.data(), GL_STATIC_DRAW);

    // Set vertex coordinates to location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Set vertex normals to location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set vertex texture coordinates to location 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Table Model
    Model model3(":/models/tabletext.obj");
    model3.unitize();
    QVector<float> tableData = model3.getVNTInterleaved();

    this->tableSize = model3.getVertices().size();

    // Generate VAO
    glGenVertexArrays(1, &tableVAO);
    glBindVertexArray(tableVAO);

    // Generate VBO
    glGenBuffers(1, &tableVBO);
    glBindBuffer(GL_ARRAY_BUFFER, tableVBO);

    // Write the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, tableData.size() * sizeof(float), tableData.data(), GL_STATIC_DRAW);

    // Set vertex coordinates to location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Set vertex normals to location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set vertex texture coordinates to location 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Empty the buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void MainView::loadTextures()
{
    glGenTextures(1, &blueTexturePtr);
    loadTexture(":/textures/blue2.png", blueTexturePtr);

    glGenTextures(1, &yellowTexturePtr);
    loadTexture(":/textures/yellow.png", yellowTexturePtr);

    glGenTextures(1, &redTexturePtr);
    loadTexture(":/textures/red.png", redTexturePtr);

    glGenTextures(1, &woodTexturePtr);
    loadTexture(":/textures/wood.png", woodTexturePtr);

}

void MainView::loadTexture(QString file, GLuint texturePtr)
{
    // Set texture parameters.
    glBindTexture(GL_TEXTURE_2D, texturePtr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Push image data to texture.
    QImage image(file);
    QVector<quint8> imageData = imageToBytes(image);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
}

// --- OpenGL drawing

/**
 * @brief MainView::paintGL
 *
 * Actual function used for drawing to the screen
 *
 */
void MainView::clearBoard(){
    diskCount = 0;

    for(int i = 0; i < 7; i++){
        columnCount[i] = 0;
    }
}

void MainView::drawObject(GLuint texturePtr, GLuint VAO, GLuint size, QMatrix4x4 objectTransform)
{

    switch (currentShader) {
    case NORMAL:
        updateNormalUniforms(objectTransform, objectTransform.normalMatrix());
        break;
    case GOURAUD:
        updateGouraudUniforms(objectTransform, objectTransform.normalMatrix());
        break;
    case PHONG:
        updatePhongUniforms(objectTransform, objectTransform.normalMatrix());
        break;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texturePtr);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, size);
}

void MainView::dropDisk(int column){
    if(columnCount[column - 1] < 6){
        //calculations
        float x = (column - 4) * 0.58;
        float y = (columnCount[column - 1] * 0.42) - 1;
        int keyFrameNumber = frameNumber;

        if(yellowPlayer){
            //drop yellow disk
            disks[diskCount] = Disk(keyFrameNumber, x, y, true);
        } else {
            //drop red disk
            disks[diskCount] = Disk(keyFrameNumber, x, y, false);
        }
        //change player
        yellowPlayer = !yellowPlayer;
        //increment column count
        columnCount[column - 1] += 1;
        //increment disk count
        diskCount += 1;
    } else {
        qDebug() << "Column " << column << " is full." ;
    }
}

void MainView::paintGL() {
    // Clear the screen before rendering
    glClearColor(0.2f, 0.5f, 0.7f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    this->updateModelTransforms();

    // Choose the selected shader.
    QOpenGLShaderProgram *shaderProgram;
    switch (currentShader) {
    case NORMAL:
        shaderProgram = &normalShaderProgram;
        shaderProgram->bind();
        break;
    case GOURAUD:
        shaderProgram = &gouraudShaderProgram;
        shaderProgram->bind();
        break;
    case PHONG:
        shaderProgram = &phongShaderProgram;
        shaderProgram->bind();
        break;
    }

    // Increment frameNumber every time the world is painted
    frameNumber += 1;

    // Connect 4 board
    // Set the texture and draw the mesh.
    drawObject(blueTexturePtr, boardVAO, boardSize, boardTransform);

    // Draw every single disk
    for (int i = 0; i < diskCount; i++){
        if (disks[i].yellowDisk){
            drawObject(yellowTexturePtr, diskVAO, diskSize, diskTransforms[i]);
        } else {
            drawObject(redTexturePtr, diskVAO, diskSize, diskTransforms[i]);
        }
    }

    if (yellowPlayer) {
        drawObject(yellowTexturePtr, diskVAO, diskSize, playerDiskTransform);
    } else {
        drawObject(redTexturePtr, diskVAO, diskSize, playerDiskTransform);
    }

    // Table
    drawObject(woodTexturePtr, tableVAO, tableSize, tableTransform);

    shaderProgram->release();
}

/**
 * @brief MainView::resizeGL
 *
 * Called upon resizing of the screen
 *
 * @param newWidth
 * @param newHeight
 */
void MainView::resizeGL(int newWidth, int newHeight)
{
    Q_UNUSED(newWidth)
    Q_UNUSED(newHeight)
    updateProjectionTransform();
}

void MainView::updateNormalUniforms(QMatrix4x4 viewTransform, QMatrix3x3 normalTransform)
{
    glUniformMatrix4fv(uniformProjectionTransformNormal, 1, GL_FALSE, projectionTransform.data());
    glUniformMatrix4fv(uniformModelViewTransformNormal, 1, GL_FALSE, viewTransform.data());
    glUniformMatrix3fv(uniformNormalTransformNormal, 1, GL_FALSE, normalTransform.data());
}

void MainView::updateGouraudUniforms(QMatrix4x4 viewTransform, QMatrix3x3 normalTransform)
{
    glUniformMatrix4fv(uniformProjectionTransformGouraud, 1, GL_FALSE, projectionTransform.data());
    glUniformMatrix4fv(uniformModelViewTransformGouraud, 1, GL_FALSE, viewTransform.data());
    glUniformMatrix3fv(uniformNormalTransformGouraud, 1, GL_FALSE, normalTransform.data());

    glUniform4fv(uniformMaterialGouraud, 1, &material[0]);
    glUniform3fv(uniformLightPositionGouraud, 1, &lightPosition[0]);
    glUniform3fv(uniformLightColourGouraud, 1, &lightColour[0]);

    glUniform1i(uniformTexture1SamplerGouraud, 0); // Redundant now, but useful when you have multiple textures.

}

void MainView::updatePhongUniforms(QMatrix4x4 viewTransform, QMatrix3x3 normalTransform)
{
    glUniformMatrix4fv(uniformProjectionTransformPhong, 1, GL_FALSE, projectionTransform.data());
    glUniformMatrix4fv(uniformModelViewTransformPhong, 1, GL_FALSE, viewTransform.data());
    glUniformMatrix3fv(uniformNormalTransformPhong, 1, GL_FALSE, normalTransform.data());

    glUniform4fv(uniformMaterialPhong, 1, &material[0]);
    glUniform3fv(uniformLightPositionPhong, 1, &lightPosition[0]);
    glUniform3fv(uniformLightColourPhong, 1, &lightColour[0]);

    glUniform1i(uniformTexture1SamplerGouraud, 0);
}

void MainView::updateProjectionTransform()
{
    float aspect_ratio = static_cast<float>(width()) / static_cast<float>(height());
    projectionTransform.setToIdentity();
    projectionTransform.perspective(60, aspect_ratio, 0.2, 20);

    // Camera rotation around a point in the world (0, 0, -6)
    projectionTransform.translate(0, 0, -6);
    projectionTransform.rotate(rotation.x(), QVector3D (1.0f,0.0f,0.0f));
    projectionTransform.rotate(rotation.y(), QVector3D (0.0f,1.0f,0.0f));
    projectionTransform.rotate(rotation.z(), QVector3D (0.0f,0.0f,1.0f));
    projectionTransform.translate(0, 0, 6);

}

void MainView::updateModelTransforms()
{
    // Connect 4 board
    boardTransform.setToIdentity();
    boardTransform.translate(0, 0, -5);
    boardTransform.scale(scale * 2);

    // Disks
    for(int i = 0; i < diskCount; i++){
        diskTransforms[i].setToIdentity();
        diskTransforms[i].translate(disks[i].x, disks[i].y + 7.2, -5);
        if (frameNumber - disks[i].keyFrameNumber <= 60){
            diskTransforms[i].translate(0,-0.12 * (frameNumber - disks[i].keyFrameNumber) , 0);
        } else {
            diskTransforms[i].translate(0.0, -7.2, 0.0);
        }
        diskTransforms[i].rotate(90.0, QVector3D(1.0f,0.0f,0.0f));
        diskTransforms[i].scale(scale * 0.2);
    }

    // Disk on table indicating whose turn it is
    playerDiskTransform.setToIdentity();
    playerDiskTransform.translate(-1.5, -1.3, -4.0);
    playerDiskTransform.scale(scale * 0.2);

    // Table
    tableTransform.setToIdentity();
    tableTransform.translate(0.0, -3.2, -5);
    tableTransform.rotate(90.0, QVector3D(0.0f,1.0f,0.0f));
    tableTransform.scale(scale * 4);

    update();
}

// --- OpenGL cleanup helpers

void MainView::destroyModelBuffers()
{
    glDeleteBuffers(1, &boardVBO);
    glDeleteVertexArrays(1, &boardVAO);

    glDeleteBuffers(1, &diskVBO);
    glDeleteVertexArrays(1, &diskVAO);

    glDeleteBuffers(1, &tableVBO);
    glDeleteVertexArrays(1, &tableVAO);
}

// --- Public interface

void MainView::setRotation(int rotateX, int rotateY, int rotateZ)
{
    rotation = { static_cast<float>(rotateX), static_cast<float>(rotateY), static_cast<float>(rotateZ) };
    updateProjectionTransform();
}

void MainView::setShadingMode(ShadingMode shading)
{
    qDebug() << "Changed shading to" << shading;
    currentShader = shading;
}


// --- Private helpers

/**
 * @brief MainView::onMessageLogged
 *
 * OpenGL logging function, do not change
 *
 * @param Message
 */
void MainView::onMessageLogged( QOpenGLDebugMessage Message ) {
    qDebug() << " → Log:" << Message;
}

