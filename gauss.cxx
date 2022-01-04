#include <fstream>
#include <iostream>
#include <istream>
#include <algorithm>
#include <sstream>
#include <string>
#include <cstring>
#include <memory>
#include <cmath>
#include <utility>

//separator kolumny w pliku csv
#define CSVDLM ';'

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
            //przekazujemy do konstruktora struktury MatrxiRow przesuniety wskaznik do danych o
            //rzad*kolumne
            return MatrixRow<T>(this->columns(),this->_data.get()+row*this->columns());
        }
        
        const MatrixRow<T> operator[](int row) const{
            if(row > this->_rows){
                throw std::invalid_argument("wiersz wiekszy niz calkowita liczba wierszy");
            }
            //przekazujemy do konstruktora struktury MatrxiRow przesuniety wskaznik do danych o
            //rzad*kolumne
            return MatrixRow<T>(this->columns(),this->_data.get()+row*this->columns());
        }
        
        /*
         * metoda odpwiedzialana za zamiane wierszy o indeksach a i b miejscami
         * parametry:
         * int a        poprawny indeks wiersza pierwszego
         * int b        poprawny indeks wiersza drugiego
         */
        bool swap_row(int a, int b){
            if(a == b){
                return true;
            }
            #if _DEBUG
            std::cout << "zamiana rzedow " << a << " i " << b << " miejscami" << std::endl;
            #endif
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
            if(a == b){
                return true;
            }
            #if _DEBUG
            std::cout << "zamiana kolumn " << a << " i " << b << " miejscami" << std::endl;
            #endif
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
         * danej kolumnie znajdujacego sie po indeksie wskaznym przez row
         * parametry:
         * int column   kolumna w jakiej najwiekszy element ma byc poszukiwany
         * int row      wiersz od ktorego ma zostac wszczete poszukiwanie
         * int stop     indeks wiersza w ktorym poszukiwanie ma zostac zakonczone
         */
        std::pair<int,int> abs_max_in_column_after(int row, int column, int stop = -1) const{
            if(row >= this->_rows){
               row = this->_rows-1; 
            }
            if(column >= this->_columns){
                column = this->_columns-1;
            }
            if(stop <= 0){
                stop = this->_rows;
            }
            
            int ret = row;
            int cur = abs((*this)[row][column]);
            
            for(int i = row+1; i < stop; ++i){
                if(abs((*this)[i][column]) > cur){
                    ret = i;
                    cur = abs((*this)[i][column]);
                }
            }
            
            return std::pair<int,int>(ret,column);
        }
        
        /*
         * metoda sluzaca poszukiwaniu maksymalnego co do wartosci bezwglednej elementu w 
         * danym wierszu znajdujacego sie po indeksie wskaznym przez column
         * parametry:
         * int row      indeks wiersza w jakim najwiekszy element ma byc poszukiwany
         * int column   indeks kolumny od ktorej ma zostac wszczete poszukiwanie
         * int stop     indeks kolumny w ktorej poszukiwanie ma zostac zakonczone
         */
        std::pair<int,int> abs_max_in_row_after(int row, int column, int stop = -1) const{
            if(column >= this->_columns){
                column = this->_columns;
            }
            if(stop <= 0){
                stop = this->_columns;
            }
            
            int ret = column;
            int cur = abs((*this)[row][column]);

            for(int i = column+1; i < stop; ++i){
                if(abs((*this)[row][i]) > cur){
                    ret = i;
                    cur = abs((*this)[row][i]);
                }
            }
            
            return std::pair<int,int>(row,ret);
        }
        
        /*
         * metoda sluzaca poszukiwaniu maksymalnego co do wartosci bezwglednej elementu w 
         * pod macierzy zaczynajacej sie od row_start,column_start a konczacej na row_end,column_end
         * parametry:
         * int row_start        indeks wiersza w jakim najwiekszy element ma byc poszukiwany
         * int column_start     indeks kolumny od ktorej ma zostac rozpoczete poszukiwanie
         * int row_end          indeks wiersza w ktorym poszukiwanie ma zostac zakonczone
         * int column_end       indeks kolumny w ktorej poszukiwanie ma sie zakonczyc
         */
        std::pair<int,int> abs_max_in_sub_matrix(int row_start, int column_start, int row_end, int column_end) const{
            std::pair<int,int> ret((row_start < 0) ? 0 : row_start,(column_start < 0) ? 0 : column_start);
            int val = abs((*this)[ret.first][ret.second]);
            
            for(int i = row_start; i < row_end; ++i){
                for(int j = column_start; j < column_end; ++j){
                    if((*this)[i][j] > val){
                        val = (*this)[i][j];
                        ret.first = i;
                        ret.second = j;
                    }
                }
            }
            
            return ret;
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
/*
 * szablony funkcji poszukiwania elementu podstawowego ujednolicone co do parametrow
 * w celu wywylania ich przez wskaznik
 */
template<typename T>
void gauss_row(Matrix<T>& g, int row_start, int column_start, int row_end, int column_end){
    #if _DEBUG
    std::cout << "wybieranie najwiekszego elementu w wierszu" << std::endl;
    #endif
    std::pair<int,int> found = g.abs_max_in_row_after(row_start,column_start, g.rows());
    std::pair<int,int> old(row_start,column_start);
    if(found != old){
        g.swap_column(old.second,found.second);
    }
}

template<typename T>
void gauss_column(Matrix<T>& g, int row_start, int column_start, int row_end, int column_end){
    #if _DEBUG
    std::cout << "wybieranie najwiekszego elementu w kolumnie" << std::endl;
    #endif
    std::pair<int,int> found = g.abs_max_in_column_after(row_start,column_start, g.rows());
    std::pair<int,int> old(row_start,column_start);
    if(found != old){
        g.swap_row(old.first,found.first);
    }
}

template<typename T>
void gauss_row_column(Matrix<T>& g, int row_start, int column_start, int row_end, int column_end){
    #if _DEBUG
    std::cout << "wybieranie najwiekszego elementu w pod macierzy" << std::endl;
    #endif
    std::pair<int,int> found = g.abs_max_in_sub_matrix(row_start,column_start,row_end,column_end);
    std::pair<int,int> old(row_start,column_start);
    if(found != old){
        if(old.first != found.first){
            g.swap_row(old.first,found.first);
        }
        if(old.second != found.second){
            g.swap_column(old.second,found.second);
        }
    }
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
    
    //wskaznik do funkcji poszukiwania
    void (*chooser_func)(Matrix<T>& g, int row_start, int column_start, int row_end, int column_end) = 0;
    //wybor funkcji poszukiwania i ustawienie wskaznika
    if(chooser == ( GAUSS_CHOOSER::ROW | GAUSS_CHOOSER::COLUMN)){
        chooser_func = gauss_row_column<T>;
    }else if(chooser == GAUSS_CHOOSER::ROW){
        chooser_func = gauss_row<T>;
    }else if(chooser == GAUSS_CHOOSER::ROW){
        chooser_func = gauss_column<T>;
    }
    
    for(int i = 0; i < g.rows(); ++i){
        //jezeli funkcja poszukiwania elementu podstawowego rozna od `0` uruchomienie jej na macierzy
        if(chooser_func){ 
            chooser_func(g,i,i,g.rows(),g.rows());
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
 * parametr `chooser` umozliwia wybranie metody wybierania elementu podstawowego. Algorytm podzielony jest na 
 * kilka etapów najpierw wczytywane są macierze a i b, następnie na ich podstawie tworzona jest macierz rozszerzona,
 * ktora poddawana jest eliminacji, otrzymana w ten sposob macierz poddawana jest odwrotnej eliminacji, aby wyzerowac
 * elementy powyzej gornej przekatnej, w ostatecznym kroku wszystkie elementy na głównej przekatnej sprowadzane są do wartosci
 * `1`, tak powstale rozwiazania sa wypisywane na standardowe wyjscie.  
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
    std::cout << "========================<zadanie 1>=======================" << std::endl;
    #endif
    zadanie1("zadanie1.csv");
    
    #if _DEBUG
    std::cout << "========================<zadanie 2>=======================" << std::endl;
    #endif
    zadanie2("zadanie2.csv");
    
    #if _DEBUG
    std::cout << "========================<zadanie 3>=======================" << std::endl;
    #endif
    zadanie3("zadanie3.csv");
    
    #if _DEBUG
    std::cout << "========================<zadanie 4>=======================" << std::endl;
    #endif
    zadanie4("zadanie4.csv");
    
    return 0;
}
