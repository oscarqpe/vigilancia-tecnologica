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
void initializeKernel(float (&kernel)[66][66]) {
    for (int i = 0; i < 66; i++) {
        for (int j = 0; j < 66; j++) {
            kernel[i][j] = normalDistribution();
        }
    }
}
void showKernel(float kernel[66][66]) {
    for (int i = 0; i < 66; i++) {
        for (int j = 0; j < 66; j++) {
            cout << kernel[i][j] << "\t";
        }
        cout << endl;
    }
}
void convolution(vector<vector<int>> &matriz, float (&kernel)[66][66]) {
    float conv[735];
    float sum = 0;
    int from = 0;
    int to = 66;
    cout << "[\n";
    for (from = 0; to < 800; from++) {
        for (int i = 0; i < 66; i++)
        {
            for (int j = 0; j < 66; j++) {
                sum += matriz[i + from][j] * kernel[i][j];
            }
        }
        conv[from] = sum;
        cout << sum << ", ";
        sum = 0;
        to++;
    }
    cout << "\n]\n";
    //return conv;
}
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

    float kernel1[66][66];
    float kernel2[66][66];
    float kernel3[66][66];
    float kernel4[66][66];
    float kernel5[66][66];
    float kernel6[66][66];
    initializeKernel(kernel1);
    initializeKernel(kernel2);
    initializeKernel(kernel3);
    initializeKernel(kernel4);
    initializeKernel(kernel5);
    initializeKernel(kernel6);
    //showKernel(kernel1);
    convolution(documentosProcesar[0].matriz, kernel1);
    convolution(documentosProcesar[0].matriz, kernel2);
    convolution(documentosProcesar[0].matriz, kernel3);
    convolution(documentosProcesar[0].matriz, kernel4);
    convolution(documentosProcesar[0].matriz, kernel5);
    convolution(documentosProcesar[0].matriz, kernel6);
    //std::cin.get();
    return 0;
}

