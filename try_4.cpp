#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <map>

using namespace std;



template <class T> void Swap(T& a, T& b) {
    T c = a;
    a = b;
    b = c;
}


void BitsToFollow(int bit, unsigned short &work_byte, unsigned short &counter16, unsigned short &bits_to_follow, ofstream &file) {
    work_byte = (work_byte << 1) | (bit & 1);
    counter16++;
    if (counter16 == 16) {
        file.write((char*)&work_byte, sizeof(unsigned short));
        work_byte = 0;
        counter16 = 0;
    }
    for (bits_to_follow; bits_to_follow > 0; bits_to_follow--) {
        work_byte = (work_byte << 1) | ((~bit) & 1);
        counter16++;
        if (counter16 == 16) {
            file.write((char*)&work_byte, sizeof(unsigned short));
            work_byte = 0;
            counter16 = 0;
        }
    }
}


int main()
{
    bool quest;
    cout << "What do we do? (0 - encrypt, 1 - decrypt): ";
    cin >> quest;

    string path;
    string path2;

    cout << "Enter path to file: ";
    cin >> path;
    cout << "Enter path to file2 (Please, don't enter the same path): ";
    cin >> path2;


    if (!quest) {
        
        ifstream file_in(path);      
        if (!file_in) { cout << "Error, file does not exist!"; return 0; }
        char c;
        map <char, int> symbs_t;
        vector<unsigned short int> frequ;  vector<char> symbs;
        int size_of_file = 0;
        while ((c = file_in.get())!=-1){    // если поймали символ конца файла
                        // то в c = file.get() будет лежать -1
            symbs_t[(unsigned char)c] += 1;
            // symb_t - это контейнер map, он сам создаст ячейку с итератором(char c),
            // если такой нет (туда сразу записывается 0 конструктором по умолчанию)
            // если же есть, то просто к ней и обращается
            size_of_file++;
        }
        file_in.close();
        

        for (int i = 0; i < 256; i++) { 
            if (symbs_t[i]) { 
                frequ.push_back(symbs_t[i]); // разделяем наши символы и их счётчики на два характерных массива(чтобы не запутаться в итераторах для map)
                                    // всё равно далее нужно их перенумеровать для удобства алгоритма
                symbs.push_back(i);
                }
        }
        int taba_size = frequ.size();//размер таблицы
        for (int i = 0; i < taba_size-1; i++) {      /// первоначальная сортировка таблицы частот
            for (int j = 0; j < taba_size - 1 - i; j++) {
                if (frequ[i] < frequ[i + 1]) {
                    Swap(frequ[i], frequ[i + 1]);
                    Swap(symbs[i], symbs[i + 1]);
                }
            }   // сортируем по убыванию(как в учебнике)
        }
        /// в алгоритме декомпрессии используется чтениe по 2 байта(16 бит),
        /// поэтому здесь таблица в далее работа с шифром заносится так же в формате 16 бит(unsigned short int)
        vector<unsigned short int> b(taba_size + 1);
        /* Таблица
        * последний символ в таблице имеет в своей ячейке число вообще всех символов в файле
        * предыдущий отличается от него на величину, равную кол-ву повторений именно этого символа
        * таким образом b[i] = b[i+1] - frequ[i]
        * где b[last] = size_of_file(число символов в файле)
        */
        b[taba_size] = size_of_file;
        for (int i = taba_size - 1; i > 0; i--) {
            b[i] = b[i + 1] - frequ[i];
        }
        for (int i = 0; i< taba_size; i++)cout << symbs[i] << ' ' << b[i + 1] << endl;
        
        for (int i = 0, curr_next = 0; i < taba_size; i++) { 
            symbs_t[(unsigned char)symbs[i]] = i + 1;   
        }
        b[0] = 0;
        
        //return 0;
        // 0-й символ всегда имеет в таблице 0
        //(поэтому нумерация идёт с единицы в цикле выше(b[i+1] = ...))

        ofstream file_out(path2, ios::binary); 
             
        
        // записываем таблицу в файл
        file_out.write((char*)&taba_size, sizeof(unsigned short));//выводим размер таблицы
        for (int i = 0; i < taba_size; i++) {
            file_out << symbs[i];       // вывод в файл таблицы частот
            file_out.write((char*)&(b[i + 1]), sizeof(unsigned short));
            // только file.write() позволяет записывать в файл сразу несколько байт 
        }
        

        //=====================================================//
        // == == == == C R Y P T I N G   P H A S E == == == == //
        //=====================================================//
        // взято из учебника, что на лекциях указывался
        // добавлено от себя лишь то, что в декомпрессии используется 
        // считывание по 2 байта(Read16bit), а запись по битам от bitstofollow
        // поэтому подогнал под общий макет: вывод\ввод по 2 байта, что бы не было
        // "неровностей" (которые у меня и вышли, кстати, в первой попытке)
        unsigned short int l = 0, h = 65535;
        unsigned short int prev_l, prev_h;
        unsigned short int delitel = b[taba_size];
        
        unsigned short int First_qtr = (h + 1) / 4; // 16384
        unsigned short int Half = First_qtr * 2; // 32768
        unsigned short int Third_qtr = First_qtr * 3;// 49152
        
        unsigned short int bits_to_follow = 0; // Сколько битов сбрасывать
        unsigned short int counter16 = 0, double_byte = 0;
        file_in.open(path);
        while (!file_in.eof()) { //

            file_in.get(c);
            unsigned short j = symbs_t[(unsigned char)c];
            prev_l = l;
            prev_h = h;
            l = prev_l + b[j - 1] * (prev_h - prev_l + 1) / delitel;
            h = prev_l + b[j] * (prev_h - prev_l + 1) / delitel - 1;
            for ( ;true ; ) {
                if (h < Half) {
                    BitsToFollow(0, double_byte, counter16, bits_to_follow, file_out); // BitsToFollow(0)
                }
                else if (l >= Half) {
                    BitsToFollow(1, double_byte, counter16, bits_to_follow, file_out); // BitsToFollow(1);
                    l = l - Half;
                    h = h - Half;
                }
                else if (l >= First_qtr && h < Third_qtr) {
                    bits_to_follow++;
                    l = l - First_qtr;
                    h = h - First_qtr;
                }
                else break;
                l = l << 1; // l+=l (в алгоритме отслеживание сближения l[i] и h[i] производится умножением на 2)
                h = h << 1; // h+=h в рекоммендациях по оптимизации было описано, что можно использовать битовый сдвиг(операция работает быстрее на с++)
                h++;
            }
        }
        if (counter16!=0) { // если в рабочих байтах что-то осталось - выпихиваем это в файл со сдвигом влево 
            double_byte = double_byte << (16 - counter16);
            file_out.write((char*)&double_byte, sizeof(unsigned short));
        }

        cout << endl;
        cout << "succesfully crypted in: " << path2 << endl;
        file_in.close(); file_out.close(); 
        return 0;
    }

    else {
        // *
        ifstream file_in(path, ios::binary); 
        if (!file_in) { cout << "Error, file does not exist!"; return 0; }
          

        unsigned short int taba_size;

        file_in.read((char*)&taba_size, sizeof(unsigned short int));
        // в фазе сжатия выводился размер таблицы в формате unsigned short int
        vector<char> symbs(taba_size);
        vector<unsigned short> b(taba_size + 1);
       
        // ============================================= //
        // == == == == T A B   R E A D I N G == == == == //
        // ==============================================//

        for (int i = 0; i < taba_size; i++) {    // считывание таблицы частот
            file_in.read((char*)&(symbs[i]), sizeof(char)); // чтобы флаги(если они там есть конечно) работали согласованно
                            // решил не мешать между собой file.get() и file.read(2 bytes)
                            // считываю по 2 байта, потому что изначально и записывалось 2 байта(16 бит в формате unsigned short)
            file_in.read((char*)(& (b[i + 1])), sizeof(unsigned short));
        } 
        for (int i = 0; i < taba_size; i++) {
            cout << symbs[i] << ' ' << b[i + 1] << endl;
        }
        // =======================================================//
        // == == == == D E C R Y P T I N G   P H A S E == == == ==//
        // =======================================================//

        unsigned short int l = 0, h = 65535;
        unsigned short int prev_l, prev_h;
        // l и h можно не сохранять в массивы, т.к. используются всегда
        // их соседние индексы (i и i-1)
        // поэтому можно сделать так:
        // l[i-1], h[i-1]   ==>   prev_l, prev_h 
        // l[i], h[i]   ==>   l, h
        unsigned short int delitel = b[taba_size];
        unsigned short int tab_max = b[taba_size];
        
        unsigned short int First_qtr = (h + 1) / 4; // 16384
        unsigned short int Half = First_qtr * 2; // 32768
        unsigned short int Third_qtr = First_qtr * 3;// 49152
        
        
        unsigned short int double_byte, counter16 = 0;

        unsigned short int value, frequ;
        b[0] = 0;
        ofstream file_out(path2);
        file_in.read((char*)&value, sizeof(unsigned short));
        for (unsigned short i = 1, j; i <= tab_max; i++) {
            
            prev_l = l;
            prev_h = h;
            frequ = ((value - prev_l + 1) * delitel - 1) / (prev_h - prev_l + 1);

            for (j = 1; b[j] <= frequ; j++); 
            l = prev_l + b[j - 1] * (prev_h - prev_l + 1) / delitel;
            h = prev_l + b[j] * (prev_h - prev_l + 1) / delitel - 1;
            for ( ; true ; ) {
                if (h < Half) {}
                else if (l >= Half) {
                    value = value -  Half;
                    l = l - Half;
                    h = h - Half;
                }
                else if (l >= First_qtr && h < Third_qtr) {
                    l = l - First_qtr;
                    h = h - First_qtr;
                    value = value - First_qtr;
                }
                else break;
                l <<= 1;
                h <<= 1;
                h++;
                if (counter16) {
                    /* value = value + file.ReadBit();
                    * 2 рабочих(текущих прочитанных) байта лежит в double_byte
                    * поэтому оттуда достаём биты (слева направо, как и прочитали с файла)
                    * чтобы достать текущий(i-й) бит нужно сместить битовым сдвигом этот бит вправо
                    * и сделать конъюнкцию результата с единицей, тогда получим 0, если бит=0
                    * и единицу, если бит=1
                    */
                    value <<= 1;
                    unsigned short int readen_bit = (double_byte >> (counter16 - 1) & 1);
                    value |= readen_bit; // value = file.readbit
                    counter16--;
                }
                else { //file.read() вернёт нам NULL, если дошли до конца
                    if (!(file_in.read((char*)&double_byte, sizeof(unsigned short)))) { double_byte = 0; }
                    counter16 = 16;
                    value <<= 1;
                    unsigned short int readen_bit = (double_byte >> (counter16 - 1) & 1);

                    value |= readen_bit;
                    counter16--;
                }
            }
            file_out << (unsigned char)symbs[j - 1];
        }
        
        cout << endl;
        cout << "file succesfully decrypted in: " << path2 << endl;
        file_in.close(); file_out.close(); 

        return 0;
       
    }
}