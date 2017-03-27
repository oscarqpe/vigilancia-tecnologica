
#include <pugixml.hpp>

#include <iostream>
#include <vector>
#include <regex>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <CImg.h>

#include "Documento.h"

using namespace pugi;
using namespace std;
using namespace cimg_library;

#define ARRAY_SIZE(array) (sizeof((array)) / sizeof((array[0])))

vector<char> vocabulario =
    {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
     't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ',', '.',
     ';', ':', '!', '?', '\'', '"', '|', '/', '_', '@', '#', '$', '%', '&', '^', '~', '+', '-', '*',
     '=', '<', '>', '(', ')', '[', ']', '{', '}'};

const int sizeVoc = 66;
int labels_size = 0;
const int salidas = 672;
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

const float learningRate = 0.5;

float pesosC1[outputSize][sizeVoc][kernel1];
float pesosC2[outputSize][outputSize][kernel2];
float pesosC3[outputSize][outputSize][kernel3];
float pesosC4[outputSize][outputSize][kernel4];
float pesosC5[outputSize][outputSize][kernel5];
float pesosC6[outputSize][outputSize][kernel6];

float convolution1  [outputSize][data_lenght - kernel1 + 1];
float convolution1Del  [outputSize][data_lenght - kernel1 + 1];
float pooling1      [outputSize][(data_lenght - kernel1 + 1) / pool1];
float pooling1Del      [outputSize][(data_lenght - kernel1 + 1) / pool1];
float convolution2  [outputSize][((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1];
float convolution2Del  [outputSize][((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1];
float pooling2      [outputSize][(((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2];
float pooling2Del      [outputSize][(((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2];
float convolution3  [outputSize][((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1];
float convolution3Del  [outputSize][((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1];
float convolution4  [outputSize][(((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1];
float convolution4Del  [outputSize][(((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1];
float convolution5  [outputSize][((((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1)
                                    - kernel5 + 1];
float convolution5Del  [outputSize][((((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1)
                                    - kernel5 + 1];
float convolution6  [outputSize][(((((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1)
                                    - kernel5 + 1) - kernel6 + 1];
float convolution6Del  [outputSize][(((((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1)
                                    - kernel5 + 1) - kernel6 + 1];
float pooling6      [outputSize * (((((((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1)
                                    - kernel5 + 1) - kernel6 + 1) / pool6)];
float pooling6Del   [outputSize * (((((((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1)
                                    - kernel5 + 1) - kernel6 + 1) / pool6)];

// FULLY CONNECTED
float pesosF1[outputSize * (((((((((data_lenght - kernel1 + 1) / pool1) - kernel2 + 1) / pool2) - kernel3 + 1) - kernel4 + 1)
                            - kernel5 + 1) - kernel6 + 1) / pool6)][1024];
float hiddenLayer1Net[1024];
float hiddenLayer1Out[1024];
float hiddenLayer1Del[1024];
float pesosF2[1024][1024];
float hiddenLayer2Net[1024];
float hiddenLayer2Out[1024];
float hiddenLayer2Del[1024];
float pesosF3[1024][salidas];
float outputLayerNet[salidas];
float outputLayerOut[salidas];
float outputLayerDel[salidas];

float target[24] = {0.01, 0.99, 0.01, 0.99, 0.01, 0.01, 0.01, 0.01, 0.99, 0.01,
                   0.01, 0.99, 0.99, 0.99, 0.01, 0.99, 0.99, 0.01, 0.01, 0.01,
                   0.01, 0.99, 0.01, 0.99};
/*float target[24] = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01,
                   0.01, 0.01, 0.01, 0.01, 0.01, 0.99, 0.01, 0.01, 0.01, 0.01,
                   0.01, 0.01, 0.01, 0.01};*/
double totalError = 0.0;

void encodeOneHot(vector<vector<float>> &matriz, string document) {
    if (document.size() > data_lenght) {
        for (int i = 0; i < data_lenght; i++)
        {
            int pos = find(vocabulario.begin(), vocabulario.end(), document.at(i)) - vocabulario.begin();
            if (pos != sizeVoc) {
                matriz[sizeVoc - pos - 1][i] = 1.0;
            }
        }
    }else {
        for (int i = 0; i < document.size(); i++)
        {
            int pos = find(vocabulario.begin(), vocabulario.end(), document.at(i)) - vocabulario.begin();
            if (pos != sizeVoc) {
                matriz[sizeVoc - pos - 1][i] = 1.0;
            }
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
    string name = "/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/cplusplus/img/img";
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
void forwardConvolution(vector<vector<float>> matriz);
void forwardFulltyConected();
void backwardFullyConected();
void backwardConvolution(vector<vector<float>> matriz);
void inicializarPesos();
void calcTotalError(Documento documento);
void showConvolution(int n);
void showResult();
float maximo (float a, float b, float c);
void showPooling(int n);
float f_signoid(float numero);
vector<int> maximoUpdateDelta(float a, float b, float c, int j, int j1, int j2);
template<typename T>
T relu(T valor) {
    if (valor < 0)
        return 0;
    else
        return valor;
}
string extractBody(pugi::xml_node text) {
    if (text.attribute("TYPE").as_string() == "BRIEF")
        return NULL;
    pugi::xml_node body = text.child("BODY");
    return body.text().as_string();
}
/*
void convolution1D(float matrix[x][], int x, int y) {
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            cout << matrix[i][j]<< " ";
        }
    }
}
*/
int main(int argc, char** argv) {
    srand(time(NULL));
    vector<Documento> documentos;
    string filename = "/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/cplusplus/reuters/reut2-000.xml", line, content;
    std::ifstream infile("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/cplusplus/reuters/labels.txt");
    string label;
    vector<string> labels;
    while (infile >> label)
    {
        labels.push_back(label);
    }
    labels_size = labels.size();
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file(filename.c_str(),
            pugi::parse_default|pugi::parse_declaration);
    int i = 0;
    pugi::xml_node lewis = doc.child("LEWIS");
    for (pugi::xml_node reuter = lewis.child("REUTERS");
         reuter;
         reuter = reuter.next_sibling("REUTERS"))
    {
        //std::cout << "reuter " << reuter.attribute("TOPICS").value() << "\n";
        pugi::xml_node text = reuter.child("TEXT");
        string contenido = extractBody(text);
        if (!contenido.empty()) {
            std::vector<float> labels_(labels_size, 0.0);
            int total_labels = 0;
            // TOPICS
            pugi::xml_node topics = reuter.child("TOPICS");
            for (pugi::xml_node d = topics.child("D");
                 d;
                 d = reuter.next_sibling("D"))
            {
                int pos = find(labels.begin(), labels.end(), d.text().as_string()) - labels.begin();
                if (pos >= 0) {
                    labels_[pos] = 1.0;
                    total_labels++;
                }
            }
            // PLACES
            pugi::xml_node places = reuter.child("PLACES");
            for (pugi::xml_node d = places.child("D"); d; d = reuter.next_sibling("D"))
            {
                int pos = find(labels.begin(), labels.end(), d.text().as_string()) - labels.begin();
                if (pos >= 0) {
                    labels_[pos] = 1.0;
                    total_labels++;
                }
            }
            // PEOPLE
            pugi::xml_node people = reuter.child("PEOPLE");
            for (pugi::xml_node d = people.child("D"); d; d = reuter.next_sibling("D"))
            {
                int pos = find(labels.begin(), labels.end(), d.text().as_string()) - labels.begin();
                if (pos >= 0) {
                    labels_[pos] = 1.0;
                    total_labels++;
                }
            }
            // ORGS
            pugi::xml_node orgs = reuter.child("ORGS");
            for (pugi::xml_node d = orgs.child("D"); d; d = reuter.next_sibling("D"))
            {
                int pos = find(labels.begin(), labels.end(), d.text().as_string()) - labels.begin();
                if (pos >= 0) {
                    labels_[pos] = 1.0;
                    total_labels++;
                }
            }
            // EXCHANGES
            pugi::xml_node exchanges = reuter.child("EXCHANGES");
            for (pugi::xml_node d = exchanges.child("D"); d; d = reuter.next_sibling("D"))
            {
                int pos = find(labels.begin(), labels.end(), d.text().as_string()) - labels.begin();
                if (pos >= 0) {
                    labels_[pos] = 1.0;
                    total_labels++;
                }
            }
            if (total_labels > 0) {
                //cout << "Texto (" << i + 1 <<"): ";
                //cout<<"Total Labels: " << total_labels << endl;
                Documento documento(data_lenght, sizeVoc);
                documento.setContenido(contenido);
                documento.setEtiquetas(labels_);
                encodeOneHot(documento.matriz, documento.getContenido());
                documentos.push_back(documento);
                i++;
            }
        }
    }

    cout<<"Total documentos leidos: "<<documentos.size()<<endl;
    float mat[2][2] = {{1,2},{3,4}};
    //convolution1D(mat, 2, 2);
    // ################ TRAIN ############## //
    inicializarPesos();
    // FORWARD CONVOLUTION
    for (int i = 0; i < 20; i++) {
        cout << "TEXTO: " << i + 1 << endl;
        forwardConvolution(documentos[i].matriz);
        forwardFulltyConected();
        calcTotalError(documentos[i]);
        //backwardFullyConected();
        //backwardConvolution(documentos[i].matriz);
        showResult();
    }

    //std::cin.get();
    return 0;
}
void showResult() {
    cout << ":::::::::::: RESULTS ::::::::::::" << endl;
    cout << "Error Total: " << totalError << endl;
}

void forwardConvolution(vector<vector<float>> matriz) {
    // convolution 1
    //cout << ":::: START CONVOLUTIONS ::::" << endl;
    //cout << "-> CONVOLUTION 1" << endl;
    //cout << "Input: " << matriz.size() <<endl;
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
                    sum += matriz[i][j + from] * pesosC1[z][i][j];
                }
            }
            convolution1[z][from] = sum < 0 ? 0 : sum;
            sum = 0;
        }
    }
    //showConvolution(1);
    // pooling 1
    //cout << "-> POOLING 1" << endl;
    int frameSize = ARRAY_SIZE(convolution1);
    int frameLenght = ARRAY_SIZE(convolution1[0]);
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0, k = 0; j < frameLenght; j+=pool1, k++) {
            pooling1[i][k] = maximo(convolution1[i][j], convolution1[i][j + 1], convolution1[i][j + 2]);
        }
    }
    //showPooling(1);
    // convolution 2
    //cout << "-> CONVOLUTION 2" << endl;
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
    //cout << "-> POOLING 2" << endl;
    frameSize = ARRAY_SIZE(convolution2);
    frameLenght = ARRAY_SIZE(convolution2[0]);
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0, k = 0; j < frameLenght; j+=pool2, k++) {
            pooling2[i][k] = maximo(convolution2[i][j], convolution2[i][j + 1], convolution2[i][j + 2]);
        }
    }
    // showPooling(2);
    // convolution 3
    //cout << "-> CONVOLUTION 3" << endl;
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
    //cout << "-> CONVOLUTION 4" << endl;
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
    //cout << "-> CONVOLUTION 5" << endl;
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
    //cout << "-> CONVOLUTION 6" << endl;
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
    //cout << "-> POOLING 6" << endl;
    frameSize = ARRAY_SIZE(convolution6);
    frameLenght = ARRAY_SIZE(convolution6[0]);
    int inputFully = 0;
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0, k = 0; j < frameLenght; j+=pool6, k++) {
            pooling6[inputFully] = maximo(convolution6[i][j], convolution6[i][j + 1], convolution6[i][j + 2]);
            inputFully++;
        }
    }
    //showPooling(6);

}
void calcTotalError(Documento documento) {
    int sizeOutput = ARRAY_SIZE(outputLayerNet);
    totalError = 0.0;
    for (int i = 0; i < sizeOutput; i++) {
        //cout << target[i] << " - " << outputLayerOut[i] << " = " << target[i] - outputLayerOut[i] << endl;
        totalError = totalError + (0.5) *
                (documento.getEtiquetas()[i] - outputLayerOut[i]) *
                (documento.getEtiquetas()[i] - outputLayerOut[i]);
        //cout << "Total: " << totalError << endl;
    }
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
    //cout << ":::: START FULLY CONNECTED ::::" << endl;
    // fully 1
    //cout << "-> HIDDEN LAYER 1" << endl;
    int inputSize = ARRAY_SIZE(pesosF1);
    int outputSize = ARRAY_SIZE(pesosF1[0]);
    float sum = 0;
    for (int i = 0; i < outputSize; i++) {
        for (int j = 0; j < inputSize; j++) {
            sum += pesosF1[j][i] * pooling6[j];
        }
        hiddenLayer1Net[i] = sum;
        hiddenLayer1Out[i] = f_signoid(sum);
        sum = 0;
    }
    // fully 2
    //cout << "-> HIDDEN LAYER 2" << endl;
    inputSize = ARRAY_SIZE(pesosF2);
    outputSize = ARRAY_SIZE(pesosF2[0]);
    sum = 0;
    for (int i = 0; i < outputSize; i++) {
        for (int j = 0; j < inputSize; j++) {
            sum += pesosF2[j][i] * hiddenLayer1Out[j];
        }
        hiddenLayer2Net[i] = sum;
        hiddenLayer2Out[i] = f_signoid(sum);
        sum = 0;
    }
    // fully 3
    //cout << "-> OUTPUT LAYER" << endl;
    inputSize = ARRAY_SIZE(pesosF3);
    outputSize = ARRAY_SIZE(pesosF3[0]);
    sum = 0;
    for (int i = 0; i < outputSize; i++) {
        for (int j = 0; j < inputSize; j++) {
            sum += pesosF3[j][i] * hiddenLayer2Out[j];
        }
        outputLayerNet[i] = sum;
        outputLayerOut[i] = f_signoid(sum);
        //cout << outputLayerOut[i] << endl;
        sum = 0;
    }
}

void backwardFullyConected() {
    //cout << ":::: START BACKWARD FULLY CONNECTED ::::" << endl;
    //::::::::::: UPDATE DELTAS ::::::::::::::::::::://
    //cout << "-> UPDATE DELTAS" << endl;
    // DELTAS OUTPUT LAYER
    int outputSize = ARRAY_SIZE(pesosF3[0]);
    for (int i = 0; i < outputSize; i++) {
        outputLayerDel[i] = -1 * (target[i] - outputLayerOut[i]) * outputLayerOut[i]
                * (1 - outputLayerOut[i]);
    }
    // DELTAS HIDDEN LAYER 2
    outputSize = ARRAY_SIZE(pesosF2[0]);
    int weightsSize = ARRAY_SIZE(pesosF3[0]);
    for (int i = 0; i < outputSize; i++) {
        float sum = 0;
        for (int j = 0; j < weightsSize; j++) {
            sum += outputLayerDel[j] * pesosF3[i][j];
        }
        hiddenLayer2Del[i] = sum * hiddenLayer2Out[i]
                * (1 - hiddenLayer2Out[i]);
    }
    // DELTAS HIDDEN LAYER 1
    outputSize = ARRAY_SIZE(pesosF1[0]);
    weightsSize = ARRAY_SIZE(pesosF2[0]);
    for (int i = 0; i < outputSize; i++) {
        float sum = 0;
        for (int j = 0; j < weightsSize; j++) {
            sum += hiddenLayer2Del[j] * pesosF2[i][j];
        }
        hiddenLayer1Del[i] = sum * hiddenLayer1Out[i]
                * (1 - hiddenLayer1Out[i]);
    }
    // DELTAS INPUT LAYER FULLY CONNECTED
    outputSize = ARRAY_SIZE(pooling6Del);
    weightsSize = ARRAY_SIZE(pesosF1[0]);
    for (int i = 0; i < outputSize; i++) {
        float sum = 0;
        for (int j = 0; j < weightsSize; j++) {
            sum += hiddenLayer1Del[j] * pesosF1[i][j];
        }
        pooling6Del[i] = sum * pooling6[i]
                * (1 - pooling6[i]);
    }
    //::::::::::: UPDATE WEIGHTS :::::::::::://
    //cout << "-> UPDATE WEIGHTS" << endl;
    // WEIGHTS HIDDEN LAYER 1
    int inputSize = ARRAY_SIZE(pesosF1);
    outputSize = ARRAY_SIZE(pesosF1[0]);
    for (int i = 0; i < inputSize; i++) {
        for (int j = 0; j < outputSize; j++) {
            pesosF1[i][j] += learningRate * -1 * pooling6[i] * hiddenLayer1Del[j];
        }
    }
    // WEIGHTS HIDDEN LAYER 2
    inputSize = ARRAY_SIZE(pesosF2);
    outputSize = ARRAY_SIZE(pesosF2[0]);
    for (int i = 0; i < inputSize; i++) {
        for (int j = 0; j < outputSize; j++) {
            pesosF2[i][j] += learningRate * -1 * hiddenLayer1Out[i] * hiddenLayer2Del[j];
        }
    }
    // WEIGHTS OUTPUT LAYER
    inputSize = ARRAY_SIZE(pesosF3);
    outputSize = ARRAY_SIZE(pesosF3[0]);
    for (int i = 0; i < inputSize; i++) {
        for (int j = 0; j < outputSize; j++) {
            pesosF3[i][j] += learningRate * -1 * hiddenLayer2Out[i] * outputLayerDel[j];
        }
    }
}

void backwardConvolution(vector<vector<float>> matriz) {
    //cout << ":::: START BACKWARD CONVOLUTIONS ::::" << endl;
    //::::::::::: UPDATE DELTAS ::::::::::::::::::::://
    //cout << "-> UPDATE DELTAS POOLING 6" << endl;
    // DELTAS POOLING 6
    // Is the same to Input's Deltas Fully Connected
    // DELTAS CONVOLUTION 6
    //cout << "-> UPDATE DELTAS CONVOLUTION 6" << endl;
    int frameSize = ARRAY_SIZE(convolution6);
    int frameLenght = ARRAY_SIZE(convolution6[0]);
    int inputFully = 0;
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0; j < frameLenght; j+=pool6) {
            vector<int> indices = maximoUpdateDelta(
                            convolution6[i][j],
                            convolution6[i][j + 1],
                            convolution6[i][j + 2],
                            j, (j + 1), (j + 2));
            convolution6Del[i][indices[0]] = pooling6Del[inputFully];
            convolution6Del[i][indices[1]] = 0;
            convolution6Del[i][indices[2]] = 0;
            inputFully++;
        }
    }
    // DELTAS CONVOLUTION 5
    //cout << "-> UPDATE DELTAS CONVOLUTION 5" << endl;
    frameSize           = ARRAY_SIZE(convolution5);
    frameLenght         = ARRAY_SIZE(convolution5[0]);
    int numeroFiltros   = ARRAY_SIZE(pesosC6);
    int numeroSalidas   = ARRAY_SIZE(pesosC6[0]);
    int numeroKernel    = ARRAY_SIZE(pesosC6[0][0]);
    inputFully = 0;
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0; j < frameLenght; j++) {
            float error = 0;
            for (int p = 0; p < numeroFiltros; p++) {
                int q = relu(j - kernel6);
                int z = j - q;
                while (q <= j - q) {
                    error += pesosC6[p][i][z] * convolution6Del[p][q];
                    q++;
                    z--;
                }
            }
            convolution5Del[i][j] = error;
        }
    }
    // UPDATE WEIGHTS CONVOLUTION 6
    //cout << "-> UPDATE WEIGHTS CONVOLUTION 6" << endl;
    for (int f = 0; f < numeroFiltros; f++) {
        for (int i = 0; i < numeroSalidas; i++) {
            for (int j = 0; j < numeroKernel; j++) {
                float error_respecto_peso = 0;
                int h = frameLenght - kernel6 + 1;
                for (int n = j, q = 0; n < h + j - 1; n++, q--) {
                    error_respecto_peso += convolution5[i][n] * convolution6Del[f][q];
                }
                pesosC6[f][i][j] = pesosC6[f][i][j] - learningRate * error_respecto_peso;
            }
        }
    }
    // DELTAS CONVOLUTION 4
    //cout << "-> UPDATE DELTAS CONVOLUTION 4" << endl;
    frameSize           = ARRAY_SIZE(convolution4);
    frameLenght         = ARRAY_SIZE(convolution4[0]);
    numeroFiltros   = ARRAY_SIZE(pesosC5);
    numeroSalidas   = ARRAY_SIZE(pesosC5[0]);
    numeroKernel    = ARRAY_SIZE(pesosC5[0][0]);
    inputFully = 0;
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0; j < frameLenght; j++) {
            float error = 0;
            for (int p = 0; p < numeroFiltros; p++) {
                int q = relu(j - kernel5);
                int z = j - q;
                int temp_q = j - q;
                while (q <= temp_q) {
                    error += pesosC5[p][i][z] * convolution5Del[p][q];
                    q++;
                    z--;
                }
            }
            convolution4Del[i][j] = error;
        }
    }
    // UPDATE WEIGHTS CONVOLUTION 5
    //cout << "-> UPDATE WEIGHTS CONVOLUTION 5" << endl;
    for (int f = 0; f < numeroFiltros; f++) {
        for (int i = 0; i < numeroSalidas; i++) {
            for (int j = 0; j < numeroKernel; j++) {
                float error_respecto_peso = 0;
                int h = frameLenght - kernel5 + 1;
                for (int n = j, q = 0; n < h + j - 1; n++, q--) {
                    error_respecto_peso += convolution4[i][n] * convolution5Del[f][q];
                }
                pesosC5[f][i][j] = pesosC5[f][i][j] - learningRate * error_respecto_peso;
            }
        }
    }
    // DELTAS CONVOLUTION 3
    //cout << "-> UPDATE DELTAS CONVOLUTION 3" << endl;
    frameSize           = ARRAY_SIZE(convolution3);
    frameLenght         = ARRAY_SIZE(convolution3[0]);
    numeroFiltros   = ARRAY_SIZE(pesosC4);
    numeroSalidas   = ARRAY_SIZE(pesosC4[0]);
    numeroKernel    = ARRAY_SIZE(pesosC4[0][0]);
    inputFully = 0;
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0; j < frameLenght; j++) {
            float error = 0;
            for (int p = 0; p < numeroFiltros; p++) {
                int q = relu(j - kernel4);
                int z = j - q;
                int temp_q = j - q;
                while (q <= temp_q) {
                    error += pesosC4[p][i][z] * convolution4Del[p][q];
                    q++;
                    z--;
                }
            }
            convolution3Del[i][j] = error;
        }
    }
    // UPDATE WEIGHTS CONVOLUTION 4
    //cout << "-> UPDATE WEIGHTS CONVOLUTION 4" << endl;
    for (int f = 0; f < numeroFiltros; f++) {
        for (int i = 0; i < numeroSalidas; i++) {
            for (int j = 0; j < numeroKernel; j++) {
                float error_respecto_peso = 0;
                int h = frameLenght - kernel4 + 1;
                for (int n = j, q = 0; n < h + j - 1; n++, q--) {
                    error_respecto_peso += convolution3[i][n] * convolution4Del[f][q];
                }
                pesosC4[f][i][j] = pesosC4[f][i][j] - learningRate * error_respecto_peso;
            }
        }
    }
    // DELTAS POOLING 2
    //cout << "-> UPDATE DELTAS POOLING 2" << endl;
    frameSize           = ARRAY_SIZE(pooling2Del);
    frameLenght         = ARRAY_SIZE(pooling2Del[0]);
    numeroFiltros   = ARRAY_SIZE(pesosC3);
    numeroSalidas   = ARRAY_SIZE(pesosC3[0]);
    numeroKernel    = ARRAY_SIZE(pesosC3[0][0]);
    inputFully = 0;
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0; j < frameLenght; j++) {
            float error = 0;
            for (int p = 0; p < numeroFiltros; p++) {
                int q = relu(j - kernel3);
                int z = j - q;
                int temp_q = j - q;
                while (q <= temp_q) {
                    error += pesosC3[p][i][z] * convolution3Del[p][q];
                    q++;
                    z--;
                }
            }
            pooling2Del[i][j] = error;
        }
    }
    // UPDATE WEIGHTS CONVOLUTION 3
    //cout << "-> UPDATE WEIGHTS CONVOLUTION 3" << endl;
    for (int f = 0; f < numeroFiltros; f++) {
        for (int i = 0; i < numeroSalidas; i++) {
            for (int j = 0; j < numeroKernel; j++) {
                float error_respecto_peso = 0;
                int h = frameLenght - kernel3 + 1;
                for (int n = j, q = 0; n < h + j - 1; n++, q--) {
                    error_respecto_peso += pooling2[i][n] * convolution3Del[f][q];
                }
                pesosC3[f][i][j] = pesosC3[f][i][j] - learningRate * error_respecto_peso;
            }
        }
    }
    // DELTAS CONVOLUTION 2
    //cout << "-> UPDATE DELTAS CONVOLUTION 2" << endl;
    frameSize = ARRAY_SIZE(convolution2);
    frameLenght = ARRAY_SIZE(convolution2[0]);
    inputFully = 0;
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0; j < frameLenght; j+=pool2) {
            vector<int> indices = maximoUpdateDelta(
                            convolution2[i][j],
                            convolution2[i][j + 1],
                            convolution2[i][j + 2],
                            j, (j + 1), (j + 2));
            convolution2Del[i][indices[0]] = pooling2Del[i][inputFully];
            convolution2Del[i][indices[1]] = 0;
            convolution2Del[i][indices[2]] = 0;
            inputFully++;
        }
    }
    // DELTAS POOLING 1
    //cout << "-> UPDATE DELTAS POOLING 1" << endl;
    frameSize           = ARRAY_SIZE(pooling1Del);
    frameLenght         = ARRAY_SIZE(pooling1Del[0]);
    numeroFiltros   = ARRAY_SIZE(pesosC2);
    numeroSalidas   = ARRAY_SIZE(pesosC2[0]);
    numeroKernel    = ARRAY_SIZE(pesosC2[0][0]);
    inputFully = 0;
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0; j < frameLenght; j++) {
            float error = 0;
            for (int p = 0; p < numeroFiltros; p++) {
                int q = relu(j - kernel2);
                int z = j - q;
                int temp_q = j - q;
                while (q <= temp_q) {
                    error += pesosC2[p][i][z] * convolution2Del[p][q];
                    q++;
                    z--;
                }
            }
            pooling1Del[i][j] = error;
        }
    }
    // UPDATE WEIGHTS CONVOLUTION 2
    //cout << "-> UPDATE WEIGHTS CONVOLUTION 2" << endl;
    for (int f = 0; f < numeroFiltros; f++) {
        for (int i = 0; i < numeroSalidas; i++) {
            for (int j = 0; j < numeroKernel; j++) {
                float error_respecto_peso = 0;
                int h = frameLenght - kernel2 + 1;
                for (int n = j, q = 0; n < h + j - 1; n++, q--) {
                    error_respecto_peso += pooling1[i][n] * convolution2Del[f][q];
                }
                pesosC2[f][i][j] = pesosC2[f][i][j] - learningRate * error_respecto_peso;
            }
        }
    }
    // DELTAS CONVOLUTION 1
    //cout << "-> UPDATE DELTAS CONVOLUTION 1" << endl;
    frameSize = ARRAY_SIZE(convolution1Del);
    frameLenght = ARRAY_SIZE(convolution1Del[0]);
    inputFully = 0;
    for (int i = 0; i < frameSize; i++) {
        for (int j = 0; j < frameLenght; j+=pool1) {
            vector<int> indices = maximoUpdateDelta(
                            convolution1[i][j],
                            convolution1[i][j + 1],
                            convolution1[i][j + 2],
                            j, (j + 1), (j + 2));
            convolution1Del[i][indices[0]] = pooling1Del[i][inputFully];
            convolution1Del[i][indices[1]] = 0;
            convolution1Del[i][indices[2]] = 0;
            inputFully++;
        }
    }
    // UPDATE WEIGHTS CONVOLUTION 1
    //cout << "-> UPDATE WEIGHTS CONVOLUTION 1" << endl;
    frameLenght         = data_lenght;
    int vocSize         = sizeVoc;
    numeroFiltros   = ARRAY_SIZE(pesosC1);
    numeroSalidas   = ARRAY_SIZE(pesosC1[0]);
    numeroKernel    = ARRAY_SIZE(pesosC1[0][0]);

    for (int f = 0; f < numeroFiltros; f++) {
        for (int i = 0; i < numeroSalidas; i++) {
            for (int j = 0; j < numeroKernel; j++) {
                float error_respecto_peso = 0;
                int h = frameLenght - kernel3 + 1;
                for (int n = j, q = 0; n < h + j - 1; n++, q--) {
                    error_respecto_peso += matriz[i][n] * convolution1Del[f][q];
                }
                pesosC1[f][i][j] = pesosC1[f][i][j] - learningRate * error_respecto_peso;
            }
        }
    }
}
void inicializarPesos() {
    cout << "Initialize weights---" << endl;
    std::ofstream out_file;
    //out_file.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weights1.txt");
    std::ifstream in_file1;
    in_file1.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weights1.txt");
    // pesos1
    int x = ARRAY_SIZE(pesosC1);
    int y = ARRAY_SIZE(pesosC1[0]);
    int z = ARRAY_SIZE(pesosC1[0][0]);
    cout<<"[" <<x<<","<<y<<","<<z<<"]\n";
    double a;
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                //pesosC1[i][j][k] = normalDistribution();
                in_file1 >> a;
                pesosC1[i][j][k] = a;
                //out_file<< pesosC1[i][j][k]<<" ";
            }
        }
    }
    //out_file<<std::endl;
    //out_file.close();
    in_file1.close();
    cout << pesosC1[0][0][0] << endl;
    cout << pesosC1[0][0][1] << endl;
    // pesos 2
    //out_file.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weights2.txt");
    in_file1.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weights2.txt");
    x = ARRAY_SIZE(pesosC2);
    y = ARRAY_SIZE(pesosC2[0]);
    z = ARRAY_SIZE(pesosC2[0][0]);
    cout<<"[" <<x<<","<<y<<","<<z<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                //pesosC2[i][j][k] = normalDistribution();
                in_file1 >> a;
                pesosC2[i][j][k] = a;
                //out_file<< pesosC2[i][j][k]<<" ";
            }
        }
    }
    //out_file<<std::endl;
    //out_file.close();
    in_file1.close();
    // pesos 3
    //out_file.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weights3.txt");
    in_file1.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weights3.txt");
    x = ARRAY_SIZE(pesosC3);
    y = ARRAY_SIZE(pesosC3[0]);
    z = ARRAY_SIZE(pesosC3[0][0]);
    cout<<"[" <<x<<","<<y<<","<<z<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                //pesosC3[i][j][k] = normalDistribution();
                in_file1 >> a;
                pesosC3[i][j][k] = a;
                //out_file<< pesosC3[i][j][k]<<" ";
            }
        }
    }
    //out_file<<std::endl;
    //out_file.close();
    in_file1.close();
    // pesos 4
    //out_file.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weights4.txt");
    in_file1.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weights4.txt");
    x = ARRAY_SIZE(pesosC4);
    y = ARRAY_SIZE(pesosC4[0]);
    z = ARRAY_SIZE(pesosC4[0][0]);
    cout<<"[" <<x<<","<<y<<","<<z<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                //pesosC4[i][j][k] = normalDistribution();
                in_file1 >> a;
                pesosC4[i][j][k] = a;
                //out_file<< pesosC4[i][j][k]<<" ";
            }
        }
    }
    //out_file<<std::endl;
    //out_file.close();
    in_file1.close();
    // pesos 5
    //out_file.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weights5.txt");
    in_file1.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weights5.txt");
    x = ARRAY_SIZE(pesosC5);
    y = ARRAY_SIZE(pesosC5[0]);
    z = ARRAY_SIZE(pesosC5[0][0]);
    cout<<"[" <<x<<","<<y<<","<<z<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                //pesosC5[i][j][k] = normalDistribution();
                in_file1 >> a;
                pesosC5[i][j][k] = a;
                //out_file<< pesosC5[i][j][k]<<" ";
            }
        }
    }
    //out_file<<std::endl;
    //out_file.close();
    in_file1.close();
    // pesos 6
    //out_file.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weights6.txt");
    in_file1.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weights6.txt");
    x = ARRAY_SIZE(pesosC6);
    y = ARRAY_SIZE(pesosC6[0]);
    z = ARRAY_SIZE(pesosC6[0][0]);
    cout<<"[" <<x<<","<<y<<","<<z<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < z; k++) {
                //pesosC6[i][j][k] = normalDistribution();
                in_file1 >> a;
                pesosC6[i][j][k] = a;
                //out_file<< pesosC6[i][j][k]<<" ";
            }
        }
    }
    //out_file<<std::endl;
    //out_file.close();
    in_file1.close();
    // pesos fully
    // persos f1
    //out_file.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weightsf1.txt");
    in_file1.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weightsf1.txt");
    x = ARRAY_SIZE(pesosF1);
    y = ARRAY_SIZE(pesosF1[0]);
    cout<<"[" <<x<<","<<y<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            //pesosF1[i][j] = normalDistribution();
            in_file1 >> a;
            pesosF1[i][j] = a;
            //out_file<< pesosF1[i][j]<<" ";
        }
    }
    //out_file<<std::endl;
    //out_file.close();
    in_file1.close();
    // persos f2
    //out_file.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weightsf2.txt");
    in_file1.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weightsf2.txt");
    x = ARRAY_SIZE(pesosF2);
    y = ARRAY_SIZE(pesosF2[0]);
    cout<<"[" <<x<<","<<y<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            //pesosF2[i][j] = normalDistribution();
            in_file1 >> a;
            pesosF2[i][j] = a;
            //out_file<< pesosF2[i][j]<<" ";
        }
    }
    //out_file<<std::endl;
    //out_file.close();
    in_file1.close();
    // persos f3
    //out_file.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weightsf3.txt");
    in_file1.open("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/textos/weightsf3.txt");
    x = ARRAY_SIZE(pesosF3);
    y = ARRAY_SIZE(pesosF3[0]);
    cout<<"[" <<x<<","<<y<<"]\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            //pesosF3[i][j] = normalDistribution();
            in_file1 >> a;
            pesosF3[i][j] = a;
            //out_file<< pesosF3[i][j]<<" ";
        }
    }
    //out_file<<std::endl;
    //out_file.close();
    in_file1.close();
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
vector<int> maximoUpdateDelta(float a, float b, float c, int j, int j1, int j2) {
    vector<int> indices;
    if (a >= b && a >= c) {
        indices.push_back(j);
        indices.push_back(j1);
        indices.push_back(j2);
    }
    else if (b >= a && b >= c) {
        indices.push_back(j1);
        indices.push_back(j);
        indices.push_back(j2);
    }
    else if (c >= a && c >= b) {
        indices.push_back(j2);
        indices.push_back(j);
        indices.push_back(j1);
    } else {
        indices.push_back(j);
        indices.push_back(j1);
        indices.push_back(j2);
    }
    return indices;
}

float f_signoid(float numero)
{
    return 1 / (1 + pow(exp(1), -1 * numero));
}
