#ifndef DOCUMENTO_H
#define DOCUMENTO_H

#include <vector>

using namespace std;

class Documento
{
private:
    string contenido;
    vector<string> etiquetas;
public:
    vector<vector<int>> matriz;
public:
    Documento();
    ~Documento();
    void setContenido(string contenido);
    string getContenido();
    void setEtiquetas(vector<string> etiquetas);
    vector<string> getEtiquetas();
    void mostrarEtiquetas();
    void mostrarMatriz(int sizeVoc);
};

Documento::Documento()
{
}

Documento::~Documento()
{
}
void Documento::setContenido(string contenido) {
    matriz.resize(contenido.size());
    this->contenido = contenido;
}
void Documento::setEtiquetas(vector<string> etiquetas) {
    this->etiquetas = etiquetas;
}
string Documento::getContenido() {
    return this->contenido;
}
vector<string> Documento::getEtiquetas() {
    return this->etiquetas;
}
void Documento::mostrarEtiquetas() {
    cout << "[";
    for (int i = 0; i < etiquetas.size(); i++)
    {
        cout << etiquetas[i];
        if (i + 1 != etiquetas.size())
            cout << ",";
    }
    cout << "]" << endl;
}
void Documento::mostrarMatriz(int sizeVoc) {
    for (int i = 0; i < this->contenido.size(); i++)
    {
        for (int j = 0; j < sizeVoc; j++)
        {
            cout << this->matriz[i][j] << " ";
        }
        cout << endl;
    }
}

#endif // DOCUMENTO_H
