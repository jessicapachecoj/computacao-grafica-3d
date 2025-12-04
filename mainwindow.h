#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QMatrix4x4>
#include "geometria.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Classe principal da aplicação. Gerencia a interface e a lógica
// de manipulação dos objetos 3D e da câmera.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Construtor
    MainWindow(QWidget *parent = nullptr);
    // Destrutor
    ~MainWindow();

    // Slots são funções conectadas aos botões da interface.
private slots:
    // Atualiza a câmera de projeção e a viewport
    void on_pushButton_atualizarCamera_clicked();
    // Atualiza qual objeto está selecionado na ComboBox
    void on_comboBox_objetos_currentIndexChanged(int index);

    // Slots para transformar o *objeto* selecionado
    void on_pushButton_transladar_clicked();
    void on_pushButton_escalar_clicked();
    void on_pushButton_rotacionar_X_clicked();
    void on_pushButton_rotacionar_Y_clicked();
    void on_pushButton_rotacionar_Z_clicked();

    // Slots para transformar a *câmera* (View Matrix)
    void on_pushButton_win_translate_clicked();
    void on_pushButton_win_scale_clicked();
    void on_pushButton_win_rotate_clicked();

    // Slot que recebe o sinal do DisplayFileWidget para aplicar rotação com o mouse
    void handleMouseRotation(double dx, double dy);

private:
    // Carrega um modelo 3D de um arquivo .obj para a cena
    void carregarObjetoOBJ(const QString& nomeArquivo, const QString& nomeObjeto);
    // Recalcula a matriz de projeção ortogonal com os valores da UI
    void atualizarProjecao();
    // Envia todas as matrizes e objetos para o widget de desenho
    void atualizarPipeline();

private:
    // Ponteiro para os elementos da interface (botões, caixas, etc.)
    Ui::MainWindow *ui;

    // Lista principal de todos os objetos 3D da cena (em coordenadas de mundo)
    QVector<ObjetoVirtual> meusObjetos;

    // Índice do objeto atualmente selecionado na ComboBox
    int objetoSelecionado = -1;

    // Área de desenho na tela (em coordenadas de pixel)
    QRect viewport;

    // Matriz de Câmera (View Matrix), controla a posição/orientação do "observador"
    QMatrix4x4 matrizCamera;

    // Matriz de Projeção, controla o "zoom" e o tipo de lente (Ortogonal)
    QMatrix4x4 matrizProjecao;

    // Parâmetros do volume de visualização da projeção ortogonal
    float ortho_left, ortho_right, ortho_bottom, ortho_top, ortho_near, ortho_far;
};

#endif // MAINWINDOW_H
