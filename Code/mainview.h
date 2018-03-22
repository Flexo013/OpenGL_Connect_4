#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "model.h"
#include "disk.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QTimer>
#include <QVector3D>
#include <QImage>
#include <QVector>
#include <memory>
#include <QMatrix4x4>

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

    QOpenGLDebugLogger *debugLogger;
    QTimer timer; // timer used for animation

    QOpenGLShaderProgram normalShaderProgram,
                         gouraudShaderProgram,
                         phongShaderProgram;

    // Uniforms for the normal shader.
    GLint uniformModelViewTransformNormal;
    GLint uniformProjectionTransformNormal;
    GLint uniformNormalTransformNormal;

    // Uniforms for the gouraud shader.
    GLint uniformModelViewTransformGouraud;
    GLint uniformProjectionTransformGouraud;
    GLint uniformNormalTransformGouraud;

    GLint uniformMaterialGouraud;
    GLint uniformLightPositionGouraud;
    GLint uniformLightColourGouraud;

    GLint uniformTexture1SamplerGouraud;

    // Uniforms for the phong shader.
    GLint uniformModelViewTransformPhong;
    GLint uniformProjectionTransformPhong;
    GLint uniformNormalTransformPhong;

    GLint uniformMaterialPhong;
    GLint uniformLightPositionPhong;
    GLint uniformLightColourPhong;

    GLint uniformTexture1SamplerPhong;

    // Buffers
    GLuint boardVAO, diskVAO, tableVAO;
    GLuint boardVBO, diskVBO, tableVBO;
    GLuint boardSize,  diskSize, tableSize;

    // Texture
    GLuint blue2TexturePtr, grey2TexturePtr, yellow2TexturePtr, red2TexturePtr, yellowTexturePtr, redTexturePtr, woodTexturePtr;

    // Transforms
    float scale = 1.f;
    QVector3D rotation;
    QMatrix4x4 projectionTransform;
    QMatrix4x4 boardTransform, diskTransforms[42], tableTransform, playerDiskTransform;

    // Phong model constants.
    QVector4D material = {0.5, 0.5, 1, 5};
    QVector3D lightPosition = {1, 100, 1};
    QVector3D lightColour = {1, 1, 1};

    // Model animation constants.
    float modelRotation = 1.f;
    int rollingCatRotation = 2;
    int frameNumber = 0;

    // Game values
    Disk disks[42];
    int diskCount = 0;
    int columnCount[7] = {0,0,0,0,0,0,0};
    bool yellowPlayer = true;
    char board[6][7];
    char gameWinner = '0';


public:
    enum ShadingMode : GLuint
    {
        PHONG = 0, NORMAL, GOURAUD
    };

    MainView(QWidget *parent = 0);
    ~MainView();

    // Functions for widget input events
    void setRotation(int rotateX, int rotateY, int rotateZ);
    void setScale(int scale);
    void setShadingMode(ShadingMode shading);

    void dropDisk(int column);
    void drawObject(GLuint texturePtr, GLuint VAO, GLuint size, QMatrix4x4 objectTransform);
    void clearBoard();
    int isGameWon(int x, int y);
    
protected:
    void initializeGL();
    void resizeGL(int newWidth, int newHeight);
    void paintGL();

    // Functions for keyboard input events
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);

    // Function for mouse input events
    void mouseDoubleClickEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *ev);

private slots:
    void onMessageLogged( QOpenGLDebugMessage Message );

private:
    void createShaderProgram();
    void loadMesh();

    // Loads texture data into the buffer of texturePtr.
    void loadTextures();
    void loadTexture(QString file, GLuint blue2TexturePtr);

    void destroyModelBuffers();

    void updateProjectionTransform();
    void updateModelTransforms();

    void updateNormalUniforms(QMatrix4x4 viewTranform, QMatrix3x3 normalTransform);
    void updateGouraudUniforms(QMatrix4x4 viewTransform, QMatrix3x3 normalTransform);
    void updatePhongUniforms(QMatrix4x4 viewTransform, QMatrix3x3 normalTransform);

    void updateAnimation();

    // Useful utility method to convert image to bytes.
    QVector<quint8> imageToBytes(QImage image);

    // The current shader to use.
    ShadingMode currentShader = PHONG;

};

#endif // MAINVIEW_H
