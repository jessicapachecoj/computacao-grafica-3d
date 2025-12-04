#include "mainwindow.h"
#include <QApplication>

// Funcao principal do programa
int main(int argc, char *argv[])
{
    // Cria a aplicacao Qt - gerencia o loop de eventos e configuracoes
    QApplication a(argc, argv);

    // Cria a janela principal da aplicacao
    MainWindow w;

    // Exibe a janela principal na tela
    w.show();

    // Inicia o loop de eventos da aplicacao
    // Mantem o programa rodando ate que a janela seja fechada
    return a.exec();
}
