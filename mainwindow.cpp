#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include "displayfilewidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    matrizCamera.setToIdentity();
    matrizProjecao.setToIdentity();

    // --- Carrega os objetos 3D ---
    carregarObjetoOBJ(":/pokemon1.obj", "Pokemon 1");
    carregarObjetoOBJ(":/pokemon2.obj", "Pokemon 2");

    if (meusObjetos.size() >= 2) {
        // Translação inicial para separar os modelos
        meusObjetos[0].matrizModelo.translate(-20, 0, 0);
        meusObjetos[1].matrizModelo.translate(20, 15, 0);
    }

    // --- Criação manual das formas 2D (com cálculo explícito do centro) ---
    QVector3D p1(-5, 20, 0), p2(5, 20, 0), p3(0, 30, 0);
    QVector3D centroTriangulo = (p1 + p2 + p3) / 3.0f;
    ObjetoVirtual triangulo("Triângulo");
    triangulo.vertices.append(p1); triangulo.vertices.append(p2); triangulo.vertices.append(p3);
    triangulo.faces.append({0, 1, 2}); triangulo.centro = centroTriangulo;
    meusObjetos.append(triangulo);

    QVector3D q1(-5, -5, 0), q2(5, -5, 0), q3(5, 5, 0), q4(-5, 5, 0);
    QVector3D centroQuadrado = (q1 + q2 + q3 + q4) / 4.0f;
    ObjetoVirtual quadrado("Quadrado");
    quadrado.vertices.append(q1); quadrado.vertices.append(q2);
    quadrado.vertices.append(q3); quadrado.vertices.append(q4);
    quadrado.faces.append({0, 1, 2, 3}); quadrado.centro = centroQuadrado;
    meusObjetos.append(quadrado);

    QVector3D r1(-20, -30, 0), r2(20, -30, 0);
    QVector3D centroReta = (r1 + r2) / 2.0f;
    ObjetoVirtual reta("Reta");
    reta.vertices.append(r1); reta.vertices.append(r2);
    reta.faces.append({0, 1}); reta.centro = centroReta;
    meusObjetos.append(reta);

    // Preenche ComboBox
    for(const auto& objeto : meusObjetos) {
        ui->comboBox_objetos->addItem(objeto.nome);
    }
    if (!meusObjetos.isEmpty()) {
        ui->comboBox_objetos->setCurrentIndex(0);
        objetoSelecionado = 0;
    }

    // --- Configuração Inicial da UI (Visão) ---

    // Zoom In: Define um pequeno volume de visão para que os objetos preencham a tela
    ui->doubleSpinBox_ortho_left->setValue(-15.0);
    ui->doubleSpinBox_ortho_right->setValue(15.0);
    ui->doubleSpinBox_ortho_bottom->setValue(-10.0);
    ui->doubleSpinBox_ortho_top->setValue(10.0);

    // Near/Far para o Clipping Volumétrico
    ui->doubleSpinBox_ortho_near->setValue(-100.0);
    ui->doubleSpinBox_ortho_far->setValue(100.0);

    // **AJUSTE 1: Aumentar o Range Máximo para DoubleSpinBoxes (Mais de 2 dígitos)**
    double max_range = 9999.0;
    ui->doubleSpinBox_ortho_left->setRange(-max_range, max_range);
    ui->doubleSpinBox_ortho_right->setRange(-max_range, max_range);
    ui->doubleSpinBox_ortho_bottom->setRange(-max_range, max_range);
    ui->doubleSpinBox_ortho_top->setRange(-max_range, max_range);
    ui->doubleSpinBox_ortho_near->setRange(-max_range, max_range);
    ui->doubleSpinBox_ortho_far->setRange(-max_range, max_range);

    // Garante que todas as caixas de translação/escala também aceitem números grandes
    ui->doubleSpinBox_dx->setRange(-max_range, max_range);
    ui->doubleSpinBox_dy->setRange(-max_range, max_range);
    ui->doubleSpinBox_dz->setRange(-max_range, max_range);
    ui->doubleSpinBox_sx->setRange(-max_range, max_range);
    ui->doubleSpinBox_sy->setRange(-max_range, max_range);
    ui->doubleSpinBox_sz->setRange(-max_range, max_range);
    ui->doubleSpinBox_win_dx->setRange(-max_range, max_range);
    ui->doubleSpinBox_win_dy->setRange(-max_range, max_range);
    ui->doubleSpinBox_win_dz->setRange(-max_range, max_range);
    ui->doubleSpinBox_win_sx->setRange(-max_range, max_range);
    ui->doubleSpinBox_win_sy->setRange(-max_range, max_range);
    ui->doubleSpinBox_win_sz->setRange(-max_range, max_range);


    // **AJUSTE 2: Viewport (Moldura) com seus valores e range expandido**
    // Valores fornecidos pelo usuário: X=92, Y=100, Largura=600, Altura=500
    ui->spinBox_vp_x->setValue(92);
    ui->spinBox_vp_y->setValue(100);
    ui->spinBox_vp_largura->setValue(600);
    ui->spinBox_vp_altura->setValue(500);

    // Aumentar o Range Máximo para SpinBoxes (Mais de 2 dígitos)
    int max_int_range = 9999;
    ui->spinBox_vp_x->setRange(-max_int_range, max_int_range);
    ui->spinBox_vp_y->setRange(-max_int_range, max_int_range);
    ui->spinBox_vp_largura->setRange(0, max_int_range);
    ui->spinBox_vp_altura->setRange(0, max_int_range);

    ui->doubleSpinBox_sx->setValue(1.0);
    ui->doubleSpinBox_sy->setValue(1.0);
    ui->doubleSpinBox_sz->setValue(1.0);
    ui->doubleSpinBox_win_sx->setValue(1.0);
    ui->doubleSpinBox_win_sy->setValue(1.0);
    ui->doubleSpinBox_win_sz->setValue(1.0);

    connect(ui->radioButton_ortho, &QRadioButton::clicked, this, &MainWindow::on_pushButton_atualizarCamera_clicked);
    connect(ui->radioButton_perspective, &QRadioButton::clicked, this, &MainWindow::on_pushButton_atualizarCamera_clicked);
    connect(ui->areaDeDesenho, &DisplayFileWidget::rotationRequested, this, &MainWindow::handleMouseRotation);

    atualizarPipeline();
}

// Destrutor
MainWindow::~MainWindow()
{
    delete ui;
}

// Lê um arquivo OBJ, calcula o centro geométrico e adiciona à cena.
void MainWindow::carregarObjetoOBJ(const QString& nomeArquivo, const QString& nomeObjeto)
{
    QFile file(nomeArquivo);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Erro ao abrir arquivo:" << nomeArquivo;
        return;
    }

    ObjetoVirtual novoObjeto(nomeObjeto);
    QTextStream in(&file);
    QVector3D somaVertices(0, 0, 0);
    int numVertices = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.isEmpty()) continue;

        if (parts[0] == "v") {
            // Lê vértices e acumula para o cálculo do centro.
            float x = parts[1].toFloat();
            float y = parts[2].toFloat();
            float z = parts[3].toFloat();
            novoObjeto.vertices.append(QVector3D(x, y, z));
            somaVertices += QVector3D(x, y, z);
            numVertices++;
        } else if (parts[0] == "f") {
            // Lê índices das faces.
            QList<int> face;
            for (int i = 1; i < parts.size(); ++i) {
                QStringList indices = parts[i].split('/');
                int verticeIndex = indices[0].toInt();
                face.append(verticeIndex - 1);
            }
            novoObjeto.faces.append(face);
        }
    }

    if (numVertices > 0) {
        // Calcula o centro geométrico (pivô).
        novoObjeto.centro = somaVertices / numVertices;
    }

    file.close();
    meusObjetos.append(novoObjeto);
}

// Atualiza a matriz de projeção (Orthogonal ou Perspectiva).
void MainWindow::atualizarProjecao()
{
    ortho_left   = ui->doubleSpinBox_ortho_left->value();
    ortho_right  = ui->doubleSpinBox_ortho_right->value();
    ortho_bottom = ui->doubleSpinBox_ortho_bottom->value();
    ortho_top    = ui->doubleSpinBox_ortho_top->value();
    ortho_near   = ui->doubleSpinBox_ortho_near->value();
    ortho_far    = ui->doubleSpinBox_ortho_far->value();

    matrizProjecao.setToIdentity();

    if (ui->radioButton_perspective->isChecked()) {
        // Proteção: Near deve ser positivo para frustum.
        if (ortho_near <= 0) {
            ortho_near = 1.0;
            ui->doubleSpinBox_ortho_near->setValue(1.0);
        }
        matrizProjecao.frustum(ortho_left, ortho_right, ortho_bottom, ortho_top, ortho_near, ortho_far);
    } else {
        // Projeção Ortogonal.
        matrizProjecao.ortho(ortho_left, ortho_right, ortho_bottom, ortho_top, ortho_near, ortho_far);
    }
}

// Envia dados e matrizes ao widget de desenho e redesenha.
void MainWindow::atualizarPipeline()
{
    atualizarProjecao();

    int vp_x = ui->spinBox_vp_x->value();
    int vp_y = ui->spinBox_vp_y->value();
    int vp_largura = ui->spinBox_vp_largura->value();
    int vp_altura = ui->spinBox_vp_altura->value();
    viewport.setRect(vp_x, vp_y, vp_largura, vp_altura);

    ui->areaDeDesenho->setDisplayFile(&meusObjetos);
    ui->areaDeDesenho->setViewport(viewport);
    ui->areaDeDesenho->setViewMatrix(matrizCamera);
    ui->areaDeDesenho->setProjectionMatrix(matrizProjecao);

    ui->areaDeDesenho->update();
}

void MainWindow::on_pushButton_atualizarCamera_clicked()
{
    atualizarPipeline();
}

void MainWindow::on_comboBox_objetos_currentIndexChanged(int index)
// Atualiza o índice do objeto selecionado.
{
    objetoSelecionado = index;
}

// Aplica translação.
void MainWindow::on_pushButton_transladar_clicked()
{
    if (objetoSelecionado < 0) return;
    double dx = ui->doubleSpinBox_dx->value();
    double dy = ui->doubleSpinBox_dy->value();
    double dz = ui->doubleSpinBox_dz->value();

    meusObjetos[objetoSelecionado].matrizModelo.translate(dx, dy, dz);
    atualizarPipeline();
}

// Aplica escala em torno do centro geométrico (Pivoteamento).
void MainWindow::on_pushButton_escalar_clicked()
{
    if (objetoSelecionado < 0) return;
    ObjetoVirtual& obj = meusObjetos[objetoSelecionado];

    double sx = ui->doubleSpinBox_sx->value();
    double sy = ui->doubleSpinBox_sy->value();
    double sz = ui->doubleSpinBox_sz->value();

    // Pivoteamento: T(-Centro) * S * T(Centro).
    obj.matrizModelo.translate(-obj.centro);
    obj.matrizModelo.scale(sx, sy, sz);
    obj.matrizModelo.translate(obj.centro);

    atualizarPipeline();
}

// Aplica rotação em X em torno do centro geométrico (Pivoteamento).
void MainWindow::on_pushButton_rotacionar_X_clicked()
{
    if (objetoSelecionado < 0) return;
    ObjetoVirtual& obj = meusObjetos[objetoSelecionado];

    double angulo = ui->doubleSpinBox_angulo->value();

    // Pivoteamento: T(-Centro) * R * T(Centro).
    obj.matrizModelo.translate(-obj.centro);
    obj.matrizModelo.rotate(angulo, 1, 0, 0);
    obj.matrizModelo.translate(obj.centro);

    atualizarPipeline();
}

// Aplica rotação em Y em torno do centro geométrico (Pivoteamento).
void MainWindow::on_pushButton_rotacionar_Y_clicked()
{
    if (objetoSelecionado < 0) return;
    ObjetoVirtual& obj = meusObjetos[objetoSelecionado];

    double angulo = ui->doubleSpinBox_angulo->value();

    // Pivoteamento: T(-Centro) * R * T(Centro).
    obj.matrizModelo.translate(-obj.centro);
    obj.matrizModelo.rotate(angulo, 0, 1, 0);
    obj.matrizModelo.translate(obj.centro);

    atualizarPipeline();
}

// Aplica rotação em Z em torno do centro geométrico (Pivoteamento).
void MainWindow::on_pushButton_rotacionar_Z_clicked()
{
    if (objetoSelecionado < 0) return;
    ObjetoVirtual& obj = meusObjetos[objetoSelecionado];

    double angulo = ui->doubleSpinBox_angulo->value();

    // Pivoteamento: T(-Centro) * R * T(Centro).
    obj.matrizModelo.translate(-obj.centro);
    obj.matrizModelo.rotate(angulo, 0, 0, 1);
    obj.matrizModelo.translate(obj.centro);

    atualizarPipeline();
}

// Aplica translação na Câmera (View Matrix).
void MainWindow::on_pushButton_win_translate_clicked()
{
    double dx = ui->doubleSpinBox_win_dx->value();
    double dy = ui->doubleSpinBox_win_dy->value();
    double dz = ui->doubleSpinBox_win_dz->value();

    matrizCamera.translate(dx, dy, dz);
    atualizarPipeline();
}

// Aplica escala (Zoom) na Câmera (View Matrix).
void MainWindow::on_pushButton_win_scale_clicked()
{
    double sx = ui->doubleSpinBox_win_sx->value();
    double sy = ui->doubleSpinBox_win_sy->value();
    double sz = ui->doubleSpinBox_win_sz->value();

    matrizCamera.scale(sx, sy, sz);
    atualizarPipeline();
}

// Aplica rotação na Câmera (Navegação - View Matrix).
void MainWindow::on_pushButton_win_rotate_clicked()
{
    double angulo = ui->doubleSpinBox_win_angle->value();

    matrizCamera.rotate(angulo, 0, 0, 1);
    atualizarPipeline();
}

// Slot para aplicar rotação ao objeto selecionado a partir do arraste do mouse.
void MainWindow::handleMouseRotation(double dx, double dy)
{
    if (objetoSelecionado < 0) return;
    ObjetoVirtual& obj = meusObjetos[objetoSelecionado];

    const double sensitivity = 0.5;

    // Rotação horizontal (movimento X do mouse) -> Rotação no eixo Y do objeto.
    if (qAbs(dx) > 0) {
        // Pivoteamento T(-C) * R(Y) * T(C).
        obj.matrizModelo.translate(-obj.centro);
        obj.matrizModelo.rotate(dx * sensitivity, 0, 1, 0);
        obj.matrizModelo.translate(obj.centro);
    }

    // Rotação vertical (movimento Y do mouse) -> Rotação no eixo X do objeto.
    if (qAbs(dy) > 0) {
        // Pivoteamento T(-C) * R(X) * T(C).
        obj.matrizModelo.translate(-obj.centro);
        obj.matrizModelo.rotate(dy * sensitivity, 1, 0, 0);
        obj.matrizModelo.translate(obj.centro);
    }

    atualizarPipeline();
}
