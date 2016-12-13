#include <iostream>
#include <vector>
#include <regex>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <CImg.h>

#include "Documento.h"

using namespace std;
using namespace cimg_library;

//vector<char> vocabulario = {'a', 'e', 'i', 'o', 'u'};
vector<char> vocabulario =
    {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
     't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ',', '.',
     ';', ':', '!', '?', '\'', '"', '|', '/', '_', '@', '#', '$', '%', '&', '^', '~', '+', '-', '*',
     '=', '<', '>', '(', ')', '[', ']', '{', '}'};
int sizeVoc = vocabulario.size();

struct kernel {
    float **datos;
    float size;
    bool is1D = false;
};
struct convolution {
    float **datos;
    int x;
    int y;
};

void getAllEtiquetas(string contenido, vector<string> &allEtiquetas) {
    regex subjectRxEtiquetas("<D>(.*?)</D>", regex_constants::icase);
    regex_iterator<string::iterator> itEtiqueta(contenido.begin(), contenido.end(), subjectRxEtiquetas);
    regex_iterator<string::iterator> end;
    while (itEtiqueta != end) {
        string etiqueta = itEtiqueta->str();
        allEtiquetas.push_back(etiqueta);
        ++itEtiqueta;
    }
}
vector<string> getEtiquetas (string contenido) {
    vector<string> allEtiquetas;
    smatch OuMatches;

    regex_iterator<string::iterator> end;

    regex subjectRxTopic("<topics>(.*?)</topics>", regex_constants::icase);
    regex_iterator<string::iterator> itTopic(contenido.begin(), contenido.end(), subjectRxTopic);
    while (itTopic != end)
    {
        string etiquetas = itTopic->str();
        getAllEtiquetas(etiquetas, allEtiquetas);
        ++itTopic;
    }

    regex subjectRxPlace("<places>(.*?)</places>", regex_constants::icase);
    regex_iterator<string::iterator> itPlace(contenido.begin(), contenido.end(), subjectRxPlace);
    while (itPlace != end)
    {
        string etiquetas = itPlace->str();
        getAllEtiquetas(etiquetas, allEtiquetas);
        ++itPlace;
    }

    regex subjectRxPeople("<people>(.*?)</people>", regex_constants::icase);
    regex_iterator<string::iterator> itPeople(contenido.begin(), contenido.end(), subjectRxPeople);
    while (itPeople != end)
    {
        string etiquetas = itPeople->str();
        getAllEtiquetas(etiquetas, allEtiquetas);
        ++itPeople;
    }

    regex subjectRxOrg("<orgs>(.*?)</orgs>", regex_constants::icase);
    regex_iterator<string::iterator> itOrg(contenido.begin(), contenido.end(), subjectRxOrg);
    while (itOrg != end)
    {
        string etiquetas = itOrg->str();
        getAllEtiquetas(etiquetas, allEtiquetas);
        ++itOrg;
    }

    regex subjectRxExchange("<exchanges>(.*?)</exchanges>", regex_constants::icase);
    regex_iterator<string::iterator> itExchange(contenido.begin(), contenido.end(), subjectRxExchange);
    while (itExchange != end)
    {
        string etiquetas = itExchange->str();
        getAllEtiquetas(etiquetas, allEtiquetas);
        ++itExchange;
    }

    regex subjectRxCompany("<companies>(.*?)</companies>", regex_constants::icase);
    regex_iterator<string::iterator> itCompany(contenido.begin(), contenido.end(), subjectRxCompany);
    while (itCompany != end)
    {
        string etiquetas = itCompany->str();
        getAllEtiquetas(etiquetas, allEtiquetas);
        ++itCompany;
    }
    return allEtiquetas;
}
string getContenido(string contenido) {
    string body;
    smatch OuMatches;
    regex_iterator<string::iterator> end;
    regex subjectRxBody("<body>(.*?)</body>", regex_constants::icase);
    regex_iterator<string::iterator> itBody(contenido.begin(), contenido.end(), subjectRxBody);
    while (itBody != end)
    {
        body = itBody->str();
        ++itBody;
    }
    return body;
}
void encodeOneHot(vector<vector<int>> &matriz, string document) {
    for (int i = 0; i < document.size(); i++)
    {
        //cout << texto.at(i) << " : ";

        int pos = find(vocabulario.begin(), vocabulario.end(), document.at(i)) - vocabulario.begin();
        //cout << pos << endl;
        matriz[i].resize(sizeVoc);
        if (pos != sizeVoc) {

            matriz[i][sizeVoc - pos - 1] = 1;
        }
    }
}
void drawMatrix(int width, int height, vector<vector<int>> matrix, int id){
    CImg<float> img(width,height,1,3);
    cimg_forXYC(img,x,y,c) { img(x,y,c) = 255; }
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            if (matrix[i][j] == 1) {
                img(i,j,0,0) = 0; //R
                img(i,j,0,1) = 0; //G
                img(i,j,0,2) = 0; //B
            }
        }
    }

    //img.display("Display my image");
    std::string s = std::to_string(id);
    string name = "/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/img/img";
    name.append(s);
    name.append(".bmp");
    img.save_bmp(name.c_str());
}

/*cK define el numero de datos que se suman para obtener
la distribucion normal. Referirse al paper de generacion de
variables aleatorias. Carpeta traitement numerique de signal*/
#define cK 1000
float normalDistribution() {
    float media = 0;
    float variance = 0.02;
    float aux=0;
    float desv = sqrt(variance);
    for (int k = 1;k <= cK;k++){
        aux = aux + (float) rand() / RAND_MAX;
    }
    float num = desv * sqrt((float) 12 / cK) * (aux - (float) cK / 2) + media;
    return num;
}
void initializeKernel(kernel &_kernel) {
    int size = _kernel.size;
    if (_kernel.is1D) {
        _kernel.datos = new float*[1];
        for (int i = 0; i < 1; i++) {
            _kernel.datos[i] = new float[size];
            for (int j = 0; j < size; j++) {
                _kernel.datos[i][j] = normalDistribution();
            }
        }
    }
    else {
        _kernel.datos = new float*[size];
        for (int i = 0; i < size; i++) {
            _kernel.datos[i] = new float[size];
            for (int j = 0; j < size; j++) {
                _kernel.datos[i][j] = normalDistribution();
            }
        }
    }
}
void showKernel(kernel _kernel) {
    int size1 = _kernel.is1D ? 1 : _kernel.size;
    int size2 = _kernel.size;
    for (int i = 0; i < size1; i++)
    {
        for (int j = 0; j < size2; j++) {
            cout << _kernel.datos[i][j] << "\t";
        }
        cout << "\n";
    }
}
void doConvolution(vector<vector<int>> &matriz, kernel _kernel, convolution &_conv) {
    int stride = 1;

    float sum = 0;
    int from = 0;
    int from_y = 0;
    //cout << "[\n";
    _conv.x = sizeVoc - _kernel.size + 1;
    _conv.y = 800 - _kernel.size + 1;
    _conv.datos = new float*[_conv.x];
    _conv.datos[0] = new float[_conv.y];
    for (from = 0; /*from < 800 - _kernel.size + 1 && */(from_y <= sizeVoc - _kernel.size); from++) {
        for (int i = 0; i < _kernel.size; i++)
        {
            for (int j = 0; j < _kernel.size; j++) {
                sum += matriz[i + from][j + from_y] * _kernel.datos[i][j];
            }
        }

        //conv[from] = sum;
        _conv.datos[from_y][from] = sum;
        sum = 0;
        if (from >= 800 - _kernel.size + 1) {
            from = 0;
            from_y++;
            if (from_y <= sizeVoc - _kernel.size)
                _conv.datos[from_y] = new float[_conv.y];
        }
    }
}
void showConvolution(convolution conv) {
    for (int j = 0; j < conv.y; j++) {
        for (int i = 0; i < conv.x; i++) {
            cout << conv.datos[i][j]<< "\t";
        }
        cout << endl;
    }
}

/*float **max1DPooling(convolutions) {
    float **pooling;
    //for (int i = 0; i < 735)
}*/

int main(int argc, char** argv) {
    srand(time(NULL));

    string filename = "/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/reut2-000.sgm", line, content;

    ifstream file(filename);
    if (file.is_open()) {
        while (getline(file, line))
        {
            content += line + " ";
        }
    }
    else {
        std::cout << "Ocurrio un error al abrir el archivo: " << filename << endl;
    }
    vector<Documento> documentos;

    smatch OuMatches;
    regex subjectRx("<reuters(.*?)</reuters>", regex_constants::icase);
    //regex subjectRx("(?<=<reuters).*(?=</reuters>)", regex_constants::icase);
    regex_iterator<string::iterator> it(content.begin(), content.end(), subjectRx);
    regex_iterator<string::iterator> end;
    int i = 1;
    while (it != end)
    {
        string contenido = it->str();
        //cout << contenido << endl;
        Documento documento;
        documento.setContenido(getContenido(contenido));
        documento.setEtiquetas(getEtiquetas(contenido));
        encodeOneHot(documento.matriz, documento.getContenido());
        documentos.push_back(documento);
        ++it;
        i++;
        //if (i == 9)
        //    break;
    }
    int total = 0;
    int totalDoc = 0;
    vector<Documento> documentosProcesar;
    for (int i = 0; i < documentos.size(); i++)
    {
        if (documentos[i].getContenido().size() >= 800 && documentos[i].getEtiquetas().size() > 0) {
            cout << totalDoc + 1 << ") Texto:" << "[" << documentos[i].getEtiquetas().size() << " Etiquetas]" << endl;
            documentos[i].mostrarEtiquetas();
            //cout << documentos[i].getContenido() << endl;
            //documentos[i].mostrarMatriz(sizeVoc);
            cout << "[" << documentos[i].getContenido().size() << ", " << sizeVoc << "]" << endl;
            total +=  documentos[i].getContenido().size();
            //drawMatrix(800/*documentos[i].getContenido().size()*/, sizeVoc, documentos[i].matriz, totalDoc);
            documentosProcesar.push_back(documentos[i]);
            totalDoc++;
        }
    }
    cout << total / totalDoc << endl;

    // CONVOLUCIONES

    kernel kernel1;
    kernel1.size = 65;
    initializeKernel(kernel1);
    //float **kernel2 = initializeKernel(7, false);
    //float **kernel3 = initializeKernel(7, false);
    //float **kernel4 = initializeKernel(7, false);
    //float **kernel5 = initializeKernel(7, false);
    //float **kernel6 = initializeKernel(7, false);

    //showKernel(kernel1);

    convolution *convolutions;

    doConvolution(documentosProcesar[0].matriz, kernel1, convolutions[0]);
    /*
    convolutions[1] = convolution(documentosProcesar[0].matriz, kernel2);
    convolutions[2] = convolution(documentosProcesar[0].matriz, kernel3);
    convolutions[3] = convolution(documentosProcesar[0].matriz, kernel4);
    convolutions[4] = convolution(documentosProcesar[0].matriz, kernel5);
    convolutions[5] = convolution(documentosProcesar[0].matriz, kernel6);
    float **pooling = max1DPooling(convolutions);
    */
    showConvolution(convolutions[0]);
    //std::cin.get();
    return 0;
}

