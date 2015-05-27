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

#include <iostream>
#include "glwidget.h"

//! [0]
GlWidget::GlWidget(QWidget *parent)
    : QGLWidget(QGLFormat(/* Additional format options */), parent)
{
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
        // FIXME: these vertices encode a square big enough to cover
        // the screen with the perspective settings in
        // resizeGL(). Need a better way to let the fragment shader
        // run on all screen pixels.
        const float dx = 10;
        const float dy = 10;
        const float dz = -2;
        vertices << QVector3D(+dx, -dy, dz) << QVector3D(+dx, +dy, dz) << QVector3D(-dx, -dy, dz)
                 << QVector3D(-dx, -dy, dz) << QVector3D(+dx, +dy, dz) << QVector3D(-dx, +dy, dz);
    }
}
//! [1]

//! [2]
void GlWidget::resizeGL(int width, int height)
{
    if (height == 0) {
        height = 1;
    }

    pMatrix.setToIdentity();
    pMatrix.perspective(60.0, (float) width / (float) height, 0.001, 1000);

    glViewport(0, 0, width, height);
}
//! [2]

// For debug printing only:
static std::ostream
&operator<< (std::ostream &stream, const QVector3D &v)
{
  return stream << "{" << v.x() << ", " << v.y() << ", " << v.z() << "}";
}

//! [3]
void GlWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 mMatrix;
    QMatrix4x4 vMatrix;

    // debug:
    {
        using std::cout;
        using std::endl;
        cout << "Vertices in world and transformed coordinates:\n";
        for (QVector3D *v = vertices.begin(); v != vertices.end(); ++v)
        {
            cout << *v << "->" << (pMatrix * vMatrix * mMatrix) * (*v) << endl;
        }
    }

    shaderProgram.bind();

    {
        const QSize s = this->size();
        shaderProgram.setUniformValue("iResolution", QVector4D(s.width(), s.height(), 1, 1));
    }
    shaderProgram.setUniformValue("mvpMatrix", pMatrix * vMatrix * mMatrix);

    shaderProgram.setUniformValue("color", QColor(Qt::white));

    shaderProgram.setAttributeArray("vertex", vertices.constData());
    shaderProgram.enableAttributeArray("vertex");

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    shaderProgram.disableAttributeArray("vertex");

    shaderProgram.release();
}
//! [3]
