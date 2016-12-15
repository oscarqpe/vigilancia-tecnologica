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

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))

vector<char> vocabulario =
    {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
     't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ',', '.',
     ';', ':', '!', '?', '\'', '"', '|', '/', '_', '@', '#', '$', '%', '&', '^', '~', '+', '-', '*',
     '=', '<', '>', '(', ')', '[', ']', '{', '}'};

const int sizeVoc = 66;

const int data_lenght = 924;
const int outputSize = 256;
const int kernel1 = 7;
const int pool1 = 3;
const int kernel2 = 7;
const int pool2 = 3;
const int kernel3 = 3;
const int kernel4 = 3;
const int kernel5 = 5;
const int kernel6 = 3;
const int pool6 = 3;

float pesosC1[outputSize][sizeVoc][kernel1];
float pesosC2[outputSize][outputSize][kernel2];
float pesosC3[outputSize][outputSize][kernel3];
float pesosC4[outputSize][outputSize][kernel4];
float pesosC5[outputSize][outputSize][kernel5];
float pesosC6[outputSize][outputSize][kernel6];

float convolution1  [outputSize][data_lenght - kernel1 + 1];
float pooling1      [outputSize][(data_lenght - kernel1 + 1) / pool1];
float convolution2  [outputSize][((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1];
float pooling2      [outputSize][(((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2];
float convolution3  [outputSize][((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1];
float convolution4  [outputSize][(((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1];
float convolution5  [outputSize][((((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1)
                                    - kernel5 + 1];
float convolution6  [outputSize][(((((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1)
                                    - kernel5 + 1) - kernel6 + 1];
float pooling6      [outputSize * (((((((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1)
                                    - kernel5 + 1) - kernel6 + 1) / pool6)];

// FULLY CONNECTED
float pesosF1[outputSize * (((((((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1)
                            - kernel5 + 1) - kernel6 + 1) / pool6)][1024];
float hiddenLayer1[1024];
float pesosF2[1024][1024];
float hiddenLayer2[1024];
float pesosF3[1024][24];
float outputLayer[24];

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
void forwardConvolution(vector<vector<int>> matriz);
void forwardFulltyConected();
void backwardFullyConected();
void backwardConvolution();
void inicializarPesos();
void showConvolution(int n);
float maximo (float a, float b, float c);
void showPooling(int n);
float f_signoid(float numero);

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
        if (documentos[i].getContenido().size() >= data_lenght && documentos[i].getEtiquetas().size() > 0) {
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
    // ################ TRAIN ############## //
    inicializarPesos();
    // FORWARD CONVOLUTION

    forwardConvolution(documentos[0].matriz);
    forwardFulltyConected();
    backwardFullyConected();
    backwardConvolution();

    //std::cin.get();
    return 0;
}
void forwardConvolution(vector<vector<int>> matriz) {
    // convolution 1
    cout << "Input: " << matriz.size() <<endl;
    float sum = 0;
    int from = 0;
    int numeroFiltros = ARRAY_SIZE(pesosC1);
    int numeroSalidas = ARRAY_SIZE(pesosC1[0]);
    int numeroKernel = ARRAY_SIZE(pesosC1[0][0]);
    for (int z = 0; z < numeroFiltros; z++) {
        for (from = 0; from < data_lenght - kernel1 + 1; from++) {
            for (int i = 0; i < numeroSalidas; i++)
            {
                for (int j = 0; j < numeroKernel; j++) {
                    sum += matriz[j][i + from] * pesosC1[z][i][j];
                }
            }
            convolution1[z][from] = sum < 0 ? 0 : sum;
            sum = 0;
        }
    }
    //showConvolution(1);
    // pooling 1
    int frameSize = ARRAY_SIZE(convolution1);
    int frameLenght = ARRAY_SIZE(convolution1[0]);
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0, k = 0; j < frameLenght; j+=pool1, k++) {
            pooling1[i][k] = maximo(convolution1[i][j], convolution1[i][j + 1], convolution1[i][j + 2]);
        }
    }
    //showPooling(1);
    // convolution 2
    sum = 0;
    from = 0;
    numeroFiltros = ARRAY_SIZE(pesosC2);
    numeroSalidas = ARRAY_SIZE(pesosC2[0]);
    numeroKernel = ARRAY_SIZE(pesosC2[0][0]);
    int inputLenght = ARRAY_SIZE(pooling1[0]);
    for (int z = 0; z < numeroFiltros; z++) {
        for (from = 0; from < inputLenght - kernel2 + 1; from++) {
            for (int i = 0; i < numeroSalidas; i++)
            {
                for (int j = 0; j < numeroKernel; j++) {
                    sum += pooling1[i + from][j] * pesosC2[z][i][j];
                }
            }
            convolution2[z][from] = sum < 0 ? 0 : sum;
            sum = 0;
        }
    }
    //showConvolution(2);
    // pooling 2
    frameSize = ARRAY_SIZE(convolution2);
    frameLenght = ARRAY_SIZE(convolution2[0]);
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0, k = 0; j < frameLenght; j+=pool2, k++) {
            pooling2[i][k] = maximo(convolution2[i][j], convolution2[i][j + 1], convolution2[i][j + 2]);
        }
    }
    // showPooling(2);
    // convolution 3
    sum = 0;
    from = 0;
    numeroFiltros = ARRAY_SIZE(pesosC3);
    numeroSalidas = ARRAY_SIZE(pesosC3[0]);
    numeroKernel = ARRAY_SIZE(pesosC3[0][0]);
    inputLenght = ARRAY_SIZE(pooling2[0]);
    for (int z = 0; z < numeroFiltros; z++) {
        for (from = 0; from < inputLenght - kernel3 + 1; from++) {
            for (int i = 0; i < numeroSalidas; i++)
            {
                for (int j = 0; j < numeroKernel; j++) {
                    sum += pooling2[i + from][j] * pesosC3[z][i][j];
                }
            }
            convolution3[z][from] = sum < 0 ? 0 : sum;
            sum = 0;
        }
    }
    //showConvolution(3);
    // convolution 4
    sum = 0;
    from = 0;
    numeroFiltros = ARRAY_SIZE(pesosC4);
    numeroSalidas = ARRAY_SIZE(pesosC4[0]);
    numeroKernel = ARRAY_SIZE(pesosC4[0][0]);
    inputLenght = ARRAY_SIZE(convolution3[0]);
    for (int z = 0; z < numeroFiltros; z++) {
        for (from = 0; from < inputLenght - kernel4 + 1; from++) {
            for (int i = 0; i < numeroSalidas; i++)
            {
                for (int j = 0; j < numeroKernel; j++) {
                    sum += convolution3[i + from][j] * pesosC4[z][i][j];
                }
            }
            convolution4[z][from] = sum < 0 ? 0 : sum;
            sum = 0;
        }
    }
    //showConvolution(4);
    // convolution 5
    sum = 0;
    from = 0;
    numeroFiltros = ARRAY_SIZE(pesosC5);
    numeroSalidas = ARRAY_SIZE(pesosC5[0]);
    numeroKernel = ARRAY_SIZE(pesosC5[0][0]);
    inputLenght = ARRAY_SIZE(convolution4[0]);
    for (int z = 0; z < numeroFiltros; z++) {
        for (from = 0; from < inputLenght - kernel5 + 1; from++) {
            for (int i = 0; i < numeroSalidas; i++)
            {
                for (int j = 0; j < numeroKernel; j++) {
                    sum += convolution4[i + from][j] * pesosC5[z][i][j];
                }
            }
            convolution5[z][from] = sum < 0 ? 0 : sum;
            sum = 0;
        }
    }
    //showConvolution(5);
    // convolution 6
    sum = 0;
    from = 0;
    numeroFiltros = ARRAY_SIZE(pesosC6);
    numeroSalidas = ARRAY_SIZE(pesosC6[0]);
    numeroKernel = ARRAY_SIZE(pesosC6[0][0]);
    inputLenght = ARRAY_SIZE(convolution5[0]);
    for (int z = 0; z < numeroFiltros; z++) {
        for (from = 0; from < inputLenght - kernel6 + 1; from++) {
            for (int i = 0; i < numeroSalidas; i++)
            {
                for (int j = 0; j < numeroKernel; j++) {
                    sum += convolution5[i + from][j] * pesosC6[z][i][j];
                }
            }
            convolution6[z][from] = sum < 0 ? 0 : sum;
            sum = 0;
        }
    }
    //showConvolution(6);
    // pooling 6
    frameSize = ARRAY_SIZE(convolution6);
    frameLenght = ARRAY_SIZE(convolution6[0]);
    int inputFully = 0;
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0, k = 0; j < frameLenght; j+=pool6, k++) {
            pooling6[inputFully] = maximo(convolution6[i][j], convolution6[i][j + 1], convolution6[i][j + 2]);
            inputFully++;
        }
    }
    showPooling(6);

}

void showConvolution(int n) {
    int frameSize = 0;
    int frameLenght = 0;
    if (n == 1) {
        frameSize = ARRAY_SIZE(convolution1);
        frameLenght = ARRAY_SIZE(convolution1[0]);
    } else if (n == 2) {
        frameSize = ARRAY_SIZE(convolution2);
        frameLenght = ARRAY_SIZE(convolution2[0]);
    } else if (n == 3) {
        frameSize = ARRAY_SIZE(convolution3);
        frameLenght = ARRAY_SIZE(convolution3[0]);
    } else if (n == 4) {
        frameSize = ARRAY_SIZE(convolution4);
        frameLenght = ARRAY_SIZE(convolution4[0]);
    } else if (n == 5) {
        frameSize = ARRAY_SIZE(convolution5);
        frameLenght = ARRAY_SIZE(convolution5[0]);
    } else if (n == 6) {
        frameSize = ARRAY_SIZE(convolution6);
        frameLenght = ARRAY_SIZE(convolution6[0]);
    }
    for (int i = 0; i < frameSize; i++)
    {
        for (int j = 0; j < frameLenght; j++) {
            if (n == 1) cout << convolution1[i][j] << "\t";
            else if (n == 2) cout << convolution2[i][j] << "\t";
            else if (n == 3) cout << convolution3[i][j] << "\t";
            else if (n == 4) cout << convolution4[i][j] << "\t";
            else if (n == 5) cout << convolution5[i][j] << "\t";
            else if (n == 6) cout << convolution6[i][j] << "\t";
        }
        cout << endl;
    }
    cout<<"[" <<frameSize<<","<<frameLenght<<"]\n";
}
void showPooling(int n) {
    if (n == 6) {
        int frameSize = ARRAY_SIZE(pooling6);
        for (int i = 0; i < frameSize; i++)
        {
            cout << pooling6[i] << "\t";
        }
    } else {
        int frameSize = 0;
        int frameLenght = 0;

        if (n == 1) {
            frameSize = ARRAY_SIZE(pooling1);
            frameLenght = ARRAY_SIZE(pooling1[0]);
        } else if (n == 2) {
            frameSize = ARRAY_SIZE(pooling2);
            frameLenght = ARRAY_SIZE(pooling2[0]);
        }
        for (int i = 0; i < frameSize; i++)
        {
            for (int j = 0; j < frameLenght; j++) {
                if (n == 1) cout << pooling1[i][j] << "\t";
                else if (n == 2) cout << pooling2[i][j] << "\t";
            }
            cout << endl;
        }
    }
}

void forwardFulltyConected() {
    // fully 1
    int inputSize = ARRAY_SIZE(pesosF1);
    int outputSize = ARRAY_SIZE(pesosF1[0]);
    float sum = 0;
    for (int i = 0; i < outputSize; i++) {
        for (int j = 0; j < inputSize; j++) {
            sum += pesosF1[j][i] * pooling6[j];
        }
        hiddenLayer1[i] = f_signoid(sum);
        sum = 0;
    }
    // fully 2
    inputSize = ARRAY_SIZE(pesosF2);
    outputSize = ARRAY_SIZE(pesosF2[0]);
    sum = 0;
    for (int i = 0; i < outputSize; i++) {
        for (int j = 0; j < inputSize; j++) {
            sum += pesosF2[j][i] * hiddenLayer1[j];
        }
        hiddenLayer2[i] = f_signoid(sum);
        sum = 0;
    }
    // fully 3
    inputSize = ARRAY_SIZE(pesosF3);
    outputSize = ARRAY_SIZE(pesosF3[0]);
    sum = 0;
    for (int i = 0; i < outputSize; i++) {
        for (int j = 0; j < inputSize; j++) {
            sum += pesosF3[j][i] * hiddenLayer2[j];
        }
        outputLayer[i] = f_signoid(sum);
        cout << outputLayer[i] << endl;
        sum = 0;
    }
}

void backwardFullyConected() {

}

void backwardConvolution() {

}
void inicializarPesos() {
    // pesos1
    int x = ARRAY_SIZE(pesosC1);
    int y = ARRAY_SIZE(pesosC1[0]);
    int z = ARRAY_SIZE(pesosC1[0][0]);
    cout<<"[" <<x<<","<<y<<","<<z<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                pesosC1[i][j][k] = normalDistribution();
            }
        }
    }

    // pesos 2
    x = ARRAY_SIZE(pesosC2);
    y = ARRAY_SIZE(pesosC2[0]);
    z = ARRAY_SIZE(pesosC2[0][0]);
    cout<<"[" <<x<<","<<y<<","<<z<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                pesosC2[i][j][k] = normalDistribution();
            }
        }
    }

    // pesos 3
    x = ARRAY_SIZE(pesosC3);
    y = ARRAY_SIZE(pesosC3[0]);
    z = ARRAY_SIZE(pesosC3[0][0]);
    cout<<"[" <<x<<","<<y<<","<<z<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                pesosC3[i][j][k] = normalDistribution();
            }
        }
    }

    // pesos 4
    x = ARRAY_SIZE(pesosC4);
    y = ARRAY_SIZE(pesosC4[0]);
    z = ARRAY_SIZE(pesosC4[0][0]);
    cout<<"[" <<x<<","<<y<<","<<z<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                pesosC4[i][j][k] = normalDistribution();
            }
        }
    }
    // pesos 5
    x = ARRAY_SIZE(pesosC5);
    y = ARRAY_SIZE(pesosC5[0]);
    z = ARRAY_SIZE(pesosC5[0][0]);
    cout<<"[" <<x<<","<<y<<","<<z<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                pesosC5[i][j][k] = normalDistribution();
            }
        }
    }
    // pesos 6
    x = ARRAY_SIZE(pesosC6);
    y = ARRAY_SIZE(pesosC6[0]);
    z = ARRAY_SIZE(pesosC6[0][0]);
    cout<<"[" <<x<<","<<y<<","<<z<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                pesosC6[i][j][k] = normalDistribution();
            }
        }
    }
    // pesos fully
    // persos f1
    x = ARRAY_SIZE(pesosF1);
    y = ARRAY_SIZE(pesosF1[0]);
    cout<<"[" <<x<<","<<y<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            pesosF1[i][j] = normalDistribution();
        }
    }
    // persos f2
    x = ARRAY_SIZE(pesosF2);
    y = ARRAY_SIZE(pesosF2[0]);
    cout<<"[" <<x<<","<<y<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            pesosF2[i][j] = normalDistribution();
        }
    }
    // persos f3
    x = ARRAY_SIZE(pesosF3);
    y = ARRAY_SIZE(pesosF3[0]);
    cout<<"[" <<x<<","<<y<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            pesosF3[i][j] = normalDistribution();
        }
    }
}
float maximo (float a, float b, float c) {
    if (a >= b && a >= c) {
        return a;
    }
    else if (b >= a && b >= c) {
        return b;
    }
    else if (c >= a && c >= b) {
        return c;
    }
    return a;
}

float f_signoid(float numero)
{
    return 1 / (1 + pow(exp(1), -1 * numero));
}
