#include "displayfilewidget.h"
#include <QPainter>
#include <QPalette>
#include <QDebug>
#include <QMouseEvent>
#include <QVector3D> // Garante que QVector3D seja reconhecido

// Construtor
DisplayFileWidget::DisplayFileWidget(QWidget *parent) : QWidget(parent)
{
    // Configura o fundo do widget para ser branco
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);
    m_viewport.setRect(0, 0, 800, 600);
}

// Setters
void DisplayFileWidget::setDisplayFile(const QVector<ObjetoVirtual> *objetos) { displayFilePtr = objetos; }
void DisplayFileWidget::setViewport(const QRect &vp) { m_viewport = vp; }
void DisplayFileWidget::setViewMatrix(const QMatrix4x4 &matrix) { m_viewMatrix = matrix; }
void DisplayFileWidget::setProjectionMatrix(const QMatrix4x4 &matrix) { m_projectionMatrix = matrix; }

// --- Tratamento de Eventos de Mouse ---
void DisplayFileWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        lastMousePos = event->pos();
        isDragging = true;
    }
}

void DisplayFileWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging) {
        int dx = event->pos().x() - lastMousePos.x();
        int dy = event->pos().y() - lastMousePos.y();
        emit rotationRequested(dx, dy);
        lastMousePos = event->pos();
    }
}

void DisplayFileWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
    }
}

// --- Clipping 2D/3D ---

// Implementação do Clipping Volumétrico (3D) no espaço NDC [-1, 1].
bool DisplayFileWidget::clip3D(QVector3D &p1, QVector3D &p2)
{
    // 1. Clipping em X e Y usando o Cohen-Sutherland 2D existente.
    QPointF pt1(p1.x(), p1.y());
    QPointF pt2(p2.x(), p2.y());

    if (!cohenSutherlandClip(pt1, pt2)) {
        return false; // Rejeitado em X/Y
    }

    // Atualiza X/Y com os pontos clipados.
    p1.setX(pt1.x()); p1.setY(pt1.y());
    p2.setX(pt2.x()); p2.setY(pt2.y());

    // 2. Clipping em Z (Eixo de Profundidade) - Clipping Volumétrico.

    // Verificação de Trivial Rejeição em Z
    if ((p1.z() > 1.0 && p2.z() > 1.0) || (p1.z() < -1.0 && p2.z() < -1.0)) {
        return false;
    }

    // Interpolação e Clipping em Z
    auto clip_z = [&](QVector3D& p_in, QVector3D& p_out) {
        // Corta contra o plano Z=1 (Far)
        if (p_out.z() > 1.0) {
            double dz = p_out.z() - p_in.z();
            if (dz == 0.0) return false;
            double t = (1.0 - p_in.z()) / dz;
            p_out = p_in + (p_out - p_in) * t;
        }
        // Corta contra o plano Z=-1 (Near)
        else if (p_out.z() < -1.0) {
            double dz = p_out.z() - p_in.z();
            if (dz == 0.0) return false;
            double t = (-1.0 - p_in.z()) / dz;
            p_out = p_in + (p_out - p_in) * t;
        }
        return true;
    };

    // Tenta clipar as extremidades para dentro do volume [-1, 1]
    if (p1.z() > 1.0 || p1.z() < -1.0) {
        if (!clip_z(p2, p1)) return false;
    }
    if (p2.z() > 1.0 || p2.z() < -1.0) {
        if (!clip_z(p1, p2)) return false;
    }

    // Aceita
    return true;
}


// Calcula o OutCode (apenas X/Y, usado dentro do clip3D).
DisplayFileWidget::OutCode DisplayFileWidget::computeOutCode(double x, double y)
{
    OutCode code = INSIDE;
    if (x < -1.0) code = (OutCode)(code | LEFT);
    else if (x > 1.0) code = (OutCode)(code | RIGHT);
    if (y < -1.0) code = (OutCode)(code | BOTTOM);
    else if (y > 1.0) code = (OutCode)(code | TOP);
    return code;
}

// Algoritmo Cohen-Sutherland 2D (usado como auxiliar dentro do clip3D).
bool DisplayFileWidget::cohenSutherlandClip(QPointF &p1, QPointF &p2)
{
    OutCode outcode1 = computeOutCode(p1.x(), p1.y());
    OutCode outcode2 = computeOutCode(p2.x(), p2.y());
    bool accept = false;

    while (true) {
        if (!(outcode1 | outcode2)) { accept = true; break; }
        else if (outcode1 & outcode2) { break; }
        else {
            double x, y;
            OutCode outcodeOut = outcode1 ? outcode1 : outcode2;

            if (outcodeOut & TOP) {
                if (p2.y() != p1.y()) x = p1.x() + (p2.x() - p1.x()) * (1.0 - p1.y()) / (p2.y() - p1.y()); else x = p1.x();
                y = 1.0;
            } else if (outcodeOut & BOTTOM) {
                if (p2.y() != p1.y()) x = p1.x() + (p2.x() - p1.x()) * (-1.0 - p1.y()) / (p2.y() - p1.y()); else x = p1.x();
                y = -1.0;
            } else if (outcodeOut & RIGHT) {
                if (p2.x() != p1.x()) y = p1.y() + (p2.y() - p1.y()) * (1.0 - p1.x()) / (p2.x() - p1.x()); else y = p1.y();
                x = 1.0;
            } else if (outcodeOut & LEFT) {
                if (p2.x() != p1.x()) y = p1.y() + (p2.y() - p1.y()) * (-1.0 - p1.x()) / (p2.x() - p1.x()); else y = p1.y();
                x = -1.0;
            }

            if (outcodeOut == outcode1) {
                p1.setX(x); p1.setY(y);
                outcode1 = computeOutCode(p1.x(), p1.y());
            } else {
                p2.setX(x); p2.setY(y);
                outcode2 = computeOutCode(p2.x(), p2.y());
            }
        }
    }
    return accept;
}


// --- Função Principal de Desenho (Pipeline 3D) ---
void DisplayFileWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!displayFilePtr) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Limpa a tela e desenha a moldura da Viewport.
    painter.fillRect(rect(), Qt::white);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(m_viewport.adjusted(0, 0, -1, -1));

    // Configura o desenho para Wireframe (somente linhas)
    painter.setPen(QPen(Qt::blue, 1));
    painter.setBrush(Qt::NoBrush);

    for (const auto &objeto : *displayFilePtr) {
        // Composição MVP: Projeção * View * Model
        QMatrix4x4 mvp = m_projectionMatrix * m_viewMatrix * objeto.matrizModelo;

        auto draw_wireframe = [&](const QVector<QVector3D>& vertices, const QVector<QList<int>>& faces) {

            QVector<QList<int>> current_faces = faces;

            if (faces.isEmpty()) {
                // Define as arestas para formas 2D (Triângulo/Quadrado/Reta)
                current_faces.clear();
                for (int i = 0; i < vertices.size(); ++i) {
                    // O resultado do módulo é convertido para int.
                    int next_idx = static_cast<int>((i + 1) % vertices.size());
                    current_faces.append({i, next_idx});
                }
            }


            for (const auto& face_indices : current_faces) {
                // Para lidar com OBJ (faces com N vértices) e as formas 2D (que definimos como arestas [i, i+1])
                for (int i = 0; i < face_indices.size(); ++i) {

                    int index1 = face_indices[i];

                    // O resultado do módulo é convertido para int.
                    int next_face_index = static_cast<int>((i + 1) % face_indices.size());
                    int index2 = face_indices[next_face_index];

                    // Lógica original (e agora correta) para garantir índices válidos.
                    if (index1 >= vertices.size() || index2 >= vertices.size()) continue;


                    QVector3D v1 = vertices[index1];
                    QVector3D v2 = vertices[index2];

                    // Projeta os vértices 3D para o espaço NDC
                    QVector3D p1_ndc = mvp.map(v1);
                    QVector3D p2_ndc = mvp.map(v2);

                    // Aplica o clipping VOLUMÉTRICO (3D) em X, Y, e Z.
                    if (clip3D(p1_ndc, p2_ndc)) {

                        // Transforma para Viewport (pixels)
                        QPointF p1_viewport(
                            (p1_ndc.x() + 1.0) / 2.0 * m_viewport.width() + m_viewport.x(),
                            (1.0 - p1_ndc.y()) / 2.0 * m_viewport.height() + m_viewport.y()
                            );
                        QPointF p2_viewport(
                            (p2_ndc.x() + 1.0) / 2.0 * m_viewport.width() + m_viewport.x(),
                            (1.0 - p2_ndc.y()) / 2.0 * m_viewport.height() + m_viewport.y()
                            );
                        painter.drawLine(p1_viewport, p2_viewport);
                    }
                }
            }
        };

        draw_wireframe(objeto.vertices, objeto.faces);
    }
}
