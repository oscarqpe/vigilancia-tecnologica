#include <iostream>
#include <vector>
#include <regex>
#include <fstream>

#include <CImg.h>

#include "Documento.h"

using namespace std;
using namespace cimg_library;

//vector<char> vocabulario = {'a', 'e', 'i', 'o', 'u'};
vector<char> vocabulario = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
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
    string name = "/home/oscarqpe/qt-projects/CNNMLL/img/img";
    name.append(s);
    name.append(".bmp");
    img.save_bmp(name.c_str());
}

int main(int argc, char** argv) {
    string texto = "aeiou";

    string filename = "/home/oscarqpe/qt-projects/CNNMLL/textos/reut2-000.sgm", line, content;

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
        if (i == 9)
            break;
    }

    for (int i = 0; i < documentos.size(); i++)
    {
        cout << i + 1 << ") Texto:" << "[" << documentos[i].getEtiquetas().size() << " Etiquetas]" << endl;
        documentos[i].mostrarEtiquetas();
        cout << documentos[i].getContenido() << endl;
        documentos[i].mostrarMatriz(sizeVoc);
        drawMatrix(100/*documentos[i].getContenido().size()*/, sizeVoc, documentos[i].matriz, i);
    }

    //std::cin.get();
    return 0;
}
