#include <fstream>
#include <iostream>
#include <istream>
#include <algorithm>
#include <sstream>
#include <string>
#include <cstring>
#include <memory>
#include <cmath>

//separator kolumny w pliku csv
#define CSVDLM ';'
//flaga odpowiedzialna za wlaczenie wypisywania komunikatow ulatwiajacych badanie zachowania programu
#define _DEBUG 0

/*
 * wyliczenie dostepnych metod wyboru czesciowego dla wyboru pelnego nalezy podac:
 * alternatywe bitowa _GAUSS_CHOOSER::ROW | _GAUSS_CHOOSER::COLUMN 
 */
enum _GAUSS_CHOOSER{
    NONE    = 0,
    ROW     = 1,
    COLUMN  = 2
};
typedef enum _GAUSS_CHOOSER GAUSS_CHOOSER;

/*
 * szablon funkcji umozliwiajacej konwersje napisu w postaci std::string na dowolny typ liczbowy
 * do tego celu wykorzystywany jest strumien napisowy
 */
template<typename T>
T convert_to(const std::string &str){
    std::istringstream ss(str);
    T num;
    ss >> num;
    return num;
}

/*
 * szablon pomocniczej struktury danych sluzacej do dostepu do danych przechowywanych w wierszach za
 * pomoca operatora `[]`
 */
template<typename T>
struct _MatrixRow{
    int _columns;
    T* _data;
    
    /*
     * konstruktor rzedu macierzy
     * parametry:
     * int columns  liczba kolumn w rzedzie
     * T* data      odpowiednio dlugi blok danych w pamieci, co najmniej dlugosci T[columns]
     */
    _MatrixRow(int columns,T* data){
        this->_columns = columns;
        this->_data = data;
    }
    
    /*
     * przeladowany operator dostepu do skladowej [] sluzacy pobieraniu danych z kolumny `column` danego wiersza
     * paramtery:
     * int column   kolumna, z ktorej ma zostac zwrocona dana liczbowa
     */
    T& operator[](int column){
        if (column > this->_columns){
            throw std::invalid_argument("kolumna wieksza niz calkowita liczba kolumn");
        }
        
        return *(this->_data+column);
    }
    
    const T& operator[](int column) const{
        if (column > this->_columns){
            throw std::invalid_argument("kolumna wieksza niz calkowita liczba kolumn");
        }
        
        return *(this->_data+column);
    }
};

template<typename T>
using MatrixRow = struct _MatrixRow<T>;

/*
 * szablon struktury danych przechowujacej dane macierzy, liczbe rzedow i kolumn
 * zastosowanie szablonu umozliwia tworzenie macierzy przechowujacych dowolny typ danych liczbowych
 */
template<typename T>
struct _Matrix{
    public:
        int _rows;
        int _columns;
        std::shared_ptr<T> _data;
    
        /*
         * metoda zwraca calkowita liczbe wierszy dla danej macierzy
         */
        int rows() const{
            return this->_rows;
        }

        /*
         * metoda zwraca calkowita liczbe kolumn dla danej macierzy
         */
        int columns() const{
            return this->_columns;
        }
        
        /*
         * konstruktor tworzacy macierz rozszerzona z macierzy a i b,
         * nowa macierz powstaje przez dopisanie macierzy b za a `[a|b]`
         * paramtery:
         * const _Matrix<T>& a  pierwsza macierz
         * const _Matrix<T>& b  druga macierz
         */
        _Matrix(const _Matrix<T>& a, const _Matrix<T>& b){
            int rows = this->_rows = a.rows();
            int columns = this->_columns = a.columns()+1;
            
            //dane przechowywane sa we wskazniku wspoldzielonych dzieki ktoremu
            //mozliwe jest plytkie kopiowane tej struktury i przekazywanie jej przez wartosc
            this->_data = std::shared_ptr<T>((T*) new T[rows*columns]);
            memset(this->_data.get(),0,sizeof(T)*rows*columns);
            
            for(int i = 0; i < a.rows(); ++i){
                for(int j = 0; j < a.columns(); ++j){
                    (*this)[i][j] = a[i][j];
                }
            }
            
            for(int i = 0; i < a.rows(); ++i){
                (*this)[i][a.columns()] = b[i][0];
            }
        }
        
        /*
         * konstruktor tworzacy macierz zadanych wymiarow
         * parametry:
         * int rows     liczba rzedow
         * int columns  liczba kolumn w rzedzie   
         */
        _Matrix(int rows = 0,int columns = 0){
            this->_rows = rows;
            this->_columns = columns;
            if(sizeof(T)*rows*columns > 0){
                this->_data = std::shared_ptr<T>((T*) new T[rows*columns]);
                memset(this->_data.get(),0,sizeof(T)*rows*columns);
            }
            
            #if _DEBUG 
                std::cout << "utworzono macierz, z wierszy: " << rows << " kolumn: " << columns << " rozmiar bajtow:" << sizeof(T)*rows*columns << std::endl;
            #endif
        }
                
        MatrixRow<T> operator[](int row){
            if(row > this->_rows){
                throw std::invalid_argument("wiersz wiekszy niz calkowita liczba wierszy");
            }
            
            return MatrixRow<T>(this->columns(),this->_data.get()+row*this->columns());
        }
        
        const MatrixRow<T> operator[](int row) const{
            if(row > this->_rows){
                throw std::invalid_argument("wiersz wiekszy niz calkowita liczba wierszy");
            }
            
            return MatrixRow<T>(this->columns(),this->_data.get()+row*this->columns());
        }
        
        /*
         * metoda odpwiedzialana za zamiane wierszy o indeksach a i b miejscami
         * parametry:
         * int a        poprawny indeks wiersza pierwszego
         * int b        poprawny indeks wiersza drugiego
         */
        bool swap_row(int a, int b){
            if(a >= this->_rows || a < 0 || b >= this->_rows || b < 0){
                return false;
            }
            
            for(int i = 0; i < this->_columns; ++i){
                T tmp = (*this)[a][i];
                (*this)[a][i] = (*this)[b][i];
                (*this)[b][i] = tmp;
            }
            
            return true;
        }
        
        /*
         * metoda odpowiedzialana za zamiane kolumn o indeksach a i b miejscami
         * parametry:
         * int a        poprawny indeks pierwszej kolumny
         * int b        poprawny indeks drugiej kolumny
         */
        bool swap_column(int a, int b){
            if(a >= this->_columns || a < 0 || b >= this->_columns || b < 0){
                return false;
            }
            
            for(int i = 0; i < this->_rows; ++i){
                T tmp = (*this)[i][a];
                (*this)[i][a] = (*this)[i][b];
                (*this)[i][b] = tmp;
            }
            
            return true;
        }
        
        /*
         * metoda sluzaca poszukiwaniu maksymalnego co do wartosci bezwglednej elementu w 
         * danej kolumnie znajdujacego sie po indeksie wskaznym przez idx
         * parametry:
         * int column   kolumna w jakiej najwiekszy element ma byc poszukiwany
         * int idx      wiersz od ktorego ma zostac wszczete poszukiwanie
         */
        int abs_max_in_column_after(int column, int idx, int stop = -1) const{
            if(idx >= this->_rows) return -1;
            if(stop <= 0) stop = this->_rows;
            
            int ret = idx;
            int cur = abs((*this)[idx+1][column]);
            
            for(int i = idx+1; idx < this->_rows; ++i){
                if(abs((*this)[idx][column]) > cur){
                    ret = idx;
                    cur = abs((*this)[idx][column]);
                }
            }
            
            return ret;
        }
        
        /*
         * metoda sluzaca poszukiwaniu maksymalnego co do wartosci bezwglednej elementu w 
         * danym wierszu znajdujacego sie po indeksie wskaznym przez idx
         * parametry:
         * int row      indeks wiersza w jakim najwiekszy element ma byc poszukiwany
         * int idx      indeks kolumny od ktorej ma zostac wszczete poszukiwanie
         * int stop     indeks kolumny w ktorej poszukiwanie ma zostac zakonczone
         */
        int abs_max_in_row_after(int row, int idx, int stop = -1) const{
            if(idx >= this->_columns) return -1;
            if(stop <= 0) stop = this->_columns;
            
            int ret = idx;
            int cur = abs((*this)[row][idx+1]);

            for(int i = idx+1; idx < this->_rows; ++i){
                if(abs((*this)[row][idx]) > cur){
                    ret = idx;
                    cur = abs((*this)[row][idx]);
                }
            }
            
            return ret;
        }
        
        int abs_max_in_sub_matrix(){
            
            
        }
};

template<typename T>
using Matrix = struct _Matrix<T>;

/*
 * szablon funkcji odpowiedzialnej za wypisanie struktury danych na strumien
 */
template<typename T>
std::ostream& operator<<(std::ostream &strm, Matrix<T>& mat){
    for(int i = 0; i < mat.rows(); ++i){
        strm << "[ ";
        for(int j = 0; j < mat.columns(); ++j){
            strm << mat[i][j] << ' ';
        }
        strm << ']' << std::endl;
    }
    return strm;
}

/*
 * szablon funkcji odpwiedzialnej za wczytywanie macierzy z pliku csv
 * paramtery:
 * Matrix<T> &a     pierwsza macierz, ktora ma byc uzupelniona danymi
 * Matrix<T> &b     druga macierz, ktora ma byc uzupelniona danymi
 */
template<typename T>
bool read_matrices(std::string filePath, Matrix<T> &a, Matrix<T> &b){
    int rows = 0;
    int columns = 0;
    std::string word;
    std::fstream fin;
    std::string rline;
    
    fin.open(filePath, std::ios::in);
    if(fin.fail()){
        throw std::runtime_error("Nie udalo sie otworzyc pliku "+filePath);
    }
    
    //wczytanie liczby wierszy i kolumn
    getline(fin,rline);
    std::stringstream stream(rline);
    std::getline(stream,word,CSVDLM);
    
    a = Matrix<T>(rows = columns, columns = std::stoi(word));
    b = Matrix<T>(rows, 1);
    
    #if _DEBUG
        std::cout << "Wczytywanie macierzy a: " << rows << "x" << columns << " i b: " << rows << "x" << "1" << std::endl;
    #endif
        
    for(int i = 0; i < rows; ++i){
        getline(fin,rline);
        rline.erase(std::remove_if(rline.begin(),rline.end(),::isspace),rline.end());
        stream = std::stringstream(rline);
        
        #if _DEBUG
        std::cout << "wiersz:" << rline << std::endl;
        #endif
        
        for(int j = 0; j < columns; ++j){
            std::getline(stream,word,CSVDLM);
            a[i][j] = convert_to<T>(word);
        }

        //pusta kolumna
        std::getline(stream,word,CSVDLM);
        
        //wczytanie elementu macierzy b
        std::getline(stream,word,CSVDLM);
        b[i][0] = convert_to<T>(word);
    }
    
    fin.close();
    
    return true;
}

template<typename T>
void gauss_row(Matrix<T>& g, int row, int column){
    
}

template<typename T>
void gauss_column(Matrix<T>& g, int row, int column){
    
}

template<typename T>
void gauss_row_column(Matrix<T>& g, int row, int column){
    
}

/*
 * szablon funkcji odpowiedzialnej za eliminacje gaussa, czyli wyzerowanie wszystkich elementow pod glowna
 * przekatna
 * paramatery:
 * Matrix<T>& g     macierz na ktorej ma byc przeprowadzona operacja
 * int chooser      metoda wyboru elementu podstawowego -> patrz enum _GAUSS_CHOOSER
 */
template<typename T>
Matrix<T>& gauss(Matrix<T>& g, int chooser = 0){
    
    void (*chooser_func)(Matrix<T>& g, int row, int column) = 0;
    if(chooser & ( GAUSS_CHOOSER::ROW | GAUSS_CHOOSER::COLUMN)){
        chooser_func = gauss_row_column<T>;
    }else if(chooser & GAUSS_CHOOSER::ROW){
        chooser_func = gauss_row<T>;
    }else if(chooser & GAUSS_CHOOSER::ROW){
        chooser_func = gauss_column<T>;
    }
    
    for(int i = 0; i < g.rows(); ++i){
        if(chooser_func){ 
            chooser_func(g,0,0);
        }
        
        for(int j = i+1; j < g.rows(); ++j){
            if(g[i][i] == static_cast<T>(0)){
                throw std::runtime_error("proba dzielenia przez 0");
            }
            T tmp = g[j][i]/g[i][i];
            for(int k = 0; k < g.columns(); ++k){
                if(i != k){
                    T sub = tmp*g[i][k];
                    g[j][k] = g[j][k] - sub;
                } else g[j][k] = 0;
            }
        }
    }
    
    return g;
}

/*
 * szablon funkcji odpwiedzialnej za odwrotna eliminacje gaussa, gdy wszystkie elementy pod glowna
 * przekatna sa wyzerowane, zadaniem tej funkcji jest wyzerowanie elementow nad glowna przekatna
 * paramatery:
 * Matrix<T>& g     macierz ktorej elementy nad glowna przekatna maja byc wyzerowane
 */
template<typename T>
Matrix<T>& gauss_reverse(Matrix<T>& g){
    for(int i = g.rows()-1; i > 0; --i){
        for(int j = i-1; j >= 0; --j){
            if(g[i][i] == static_cast<T>(0)){
                throw std::runtime_error("proba dzielenia przez 0");
            }
            T tmp = g[j][i]/g[i][i];
            for(int k = 0; k < g.columns(); ++k){
                if ( i != k){
                    T sub = tmp*g[i][k];
                    g[j][k] = g[j][k] - sub;
                } else g[j][k] = 0;
            }
        }
    }
    
    return g;
}

/*
 * szablon funkcji odpowiedzialnej za sprawdzenie czy wszystkie elementy macierzy na przekatnej
 * sa rowne 1, jezeli nie w to miejsce wstawiana jest wartosc 1, a odpowiedni element kolumny wyrazow wolnych
 * dzielony jest przez wartosc elemntu na przekatnej
 * paramtery:
 * Matrix<T>& g     macierz ktora ma zostac sprawdzona
 */
template<typename T>
Matrix<T>& diagonal(Matrix<T>& g){
    for(int i = 0; i < g.rows(); ++i){
        if(g[i][i] == static_cast<T>(1)) continue;
        g[i][g.columns()-1] /= g[i][i];
        g[i][i] = 1;
    }
    
    return g;
}
/*
 * szablon funkcji odpowiedzialnej za algorytm gaussa, dane wejsciowe wczytywane sa z pliku
 * 
 */
template<typename T>
bool gauss_solve(const std::string& filePath, int chooser = 0){
    Matrix<T> a;
    Matrix<T> b;
    
    if(!read_matrices<T>(filePath,a,b)){
        throw std::runtime_error("nie udalo sie wczytac macierzy");
    }

    #if _DEBUG
    std::cout << a << b;
    #endif
    
    Matrix<T> c(a,b);
    #if _DEBUG
    std::cout << "macierz rozszerzona" << std::endl;
    std::cout << c;
    #endif
    
    c = gauss<T>(c, chooser);
    
    #if _DEBUG
    std::cout << "macierz rozszerzona po eliminacji" << std::endl;
    std::cout << c;
    #endif
    
    c = gauss_reverse<T>(c);

    #if _DEBUG
    std::cout << "macierz rozszerzona po odwrotnej eliminacji" << std::endl;
    std::cout << c;
    #endif
    
    c = diagonal<T>(c);
    
    #if _DEBUG
    std::cout << "wynik" << std::endl;
    #endif
    
    for(int i = 0; i < c.rows(); ++i){
        std::cout << "x" << i+1 << "=" << c[i][c.columns()-1] << std::endl;
    }
    
    return true;
}

#define zadanie1(plik) gauss_solve<double>(plik)
#define zadanie2(plik) gauss_solve<double>(plik,GAUSS_CHOOSER::ROW)
#define zadanie3(plik) gauss_solve<double>(plik,GAUSS_CHOOSER::COLUMN)
#define zadanie4(plik) gauss_solve<double>(plik,GAUSS_CHOOSER::ROW|GAUSS_CHOOSER::COLUMN)

int main(int argc, char **argv){

    #if _DEBUG
    std::cout << "zadanie 1" << std::endl;
    #endif
    zadanie1("zadanie1.csv");
    
    #if _DEBUG
    std::cout << "zadanie 2" << std::endl;
    #endif
    zadanie2("zadanie2.csv");
    
    #if _DEBUG
    std::cout << "zadanie 3" << std::endl;
    #endif
    zadanie3("zadanie3.csv");
    
    #if _DEBUG
    std::cout << "zadanie 4" << std::endl;
    #endif
    zadanie4("zadanie4.csv");
    
    return 0;
}
