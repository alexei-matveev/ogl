/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <cmath>
#include "glwidget.h"
#include <QMouseEvent>
#include <QWheelEvent>
#ifdef DEBUG
#include <iostream>
#endif

//! [0]
GlWidget::GlWidget(QWidget *parent)
    : QGLWidget(QGLFormat(/* Additional format options */), parent)
{
    alpha = 180;
    beta = -45;
    distance = 5.0;
}

GlWidget::~GlWidget()
{
}

QSize GlWidget::sizeHint() const
{
    return QSize(640, 480);
}
//! [0]

//! [1]
void GlWidget::initializeGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    qglClearColor(QColor(Qt::black));

    shaderProgram.addShaderFromSourceFile(QGLShader::Vertex, ":/vertexShader.vsh");
    shaderProgram.addShaderFromSourceFile(QGLShader::Fragment, ":/toy.fsh");
    shaderProgram.link();

    {
        // These vertices encode a square big enough to cover the
        // screen for the unit trafo matrix (not) used in the vertex
        // shader. We just need a way to let the fragment shader run
        // on all (or most of the) screen pixels.
        const float x = 1.0; // set < 1 to actually see the border
        const float y = 1.0; // set < 1 to actually see the border
        const float z = 0.5;  // < 1 not to be clipped
        vertices << QVector3D(+x, -y, z) << QVector3D(+x, +y, z) << QVector3D(-x, -y, z)
                 << QVector3D(-x, -y, z) << QVector3D(+x, +y, z) << QVector3D(-x, +y, z);
    }
}
//! [1]

//! [2]
void GlWidget::resizeGL(int width, int height)
{
    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);
}
//! [2]

#ifdef DEBUG
// For debug printing only:
static std::ostream
&operator<< (std::ostream &stream, const QVector3D &v)
{
  return stream << "{" << v.x() << ", " << v.y() << ", " << v.z() << "}";
}
#endif


static QMatrix4x4
setRotation (float alpha, float beta)
{
    QMatrix4x4 cameraTransformation;
    cameraTransformation.rotate(alpha, 0, 1, 0);
    cameraTransformation.rotate(beta, 1, 0, 0);
    return cameraTransformation;
}

static QMatrix4x4
setCamera (const QVector3D &w, const QVector3D &up)
{
    const QVector3D eye(0, 0, 0);

    QMatrix4x4 la;
    la.lookAt(eye, w, up);

    for (int j = 0; j < 3; ++j)
        for (int i = 0; i < 3; ++i)
            la(i, j) *= (i == 2? -1: 1);
            // FIXME: this is ugly ...

    return la;
}

//! [3]
void GlWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const QMatrix4x4 cameraTransformation = setRotation(alpha, beta);

    // Camera position (ray origin), up-direction and target to look at. The
    // 3-vector cameraPosition is a uniform that is used to start ray marching
    // from in the fragment shader.
    const QVector3D cameraPosition = cameraTransformation * QVector3D(0, 0, distance);
    const QVector3D cameraUpDirection = cameraTransformation * QVector3D(0, 1, 0);
    const QVector3D targetPosition(-0.5, -0.4, 0.5);
    const QMatrix4x4 cameraMatrix = setCamera (targetPosition - cameraPosition, cameraUpDirection);

    shaderProgram.bind();

    {
        const QSize s = this->size();
        shaderProgram.setUniformValue("iResolution", QVector4D(s.width(), s.height(), 1, 1));
    }
    shaderProgram.setUniformValue("cameraMatrix", cameraMatrix);
    shaderProgram.setUniformValue("cameraPosition", cameraPosition);

    shaderProgram.setAttributeArray("vertex", vertices.constData());
    shaderProgram.enableAttributeArray("vertex");

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    shaderProgram.disableAttributeArray("vertex");

    shaderProgram.release();
}


void GlWidget::mousePressEvent(QMouseEvent *event)
{
    lastMousePosition = event->pos();

    event->accept();
}


void GlWidget::mouseMoveEvent(QMouseEvent *event)
{
    int deltaX = event->x() - lastMousePosition.x();
    int deltaY = event->y() - lastMousePosition.y();

    if (event->buttons() & Qt::LeftButton)
    {
        alpha -= deltaX;
        while (alpha < 0)
            alpha += 360;
        while (alpha >= 360)
            alpha -= 360;

        beta -= deltaY;
        if (beta < -90)
            beta = -90;
        if (beta > 90)
            beta = 90;

        updateGL();
    }

    lastMousePosition = event->pos();

    event->accept();
}


void GlWidget::wheelEvent(QWheelEvent *event)
{
    int delta = event->delta();

    if (event->orientation() == Qt::Vertical)
    {
        if (delta < 0)
            distance *= 1.1;
        else if (delta > 0)
            distance *= 0.9;

        updateGL();
    }

    event->accept();
}
