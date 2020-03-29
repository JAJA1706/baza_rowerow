#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_USERS 9999
#define MAX_ADRESS_LENGTH 50

typedef struct List_Of_Parking_Places List;
typedef struct User User;
typedef struct Parking_Place Parking;
typedef struct tm Time;
typedef struct Bike Bike;
typedef enum District District;

//inicjacja funkcji
void Rent_Bike ( User*first, Bike*glowa, Parking*head );
void Add_Cash ( User*first );
void Leave_Bike ( User*first, Parking*head );
User* Add_User ( User*first );
Bike* Add_Bike ( Bike*glowa, Parking*head );
void zwolnij( Bike*glowa, Parking*head, User* first ); //zwalnianie zaalokowanej pamieci
void clean( void ); //czyszczenie bufora
void swap_bike( Bike*x, Bike*y ); //zamiana wartosci miedzy elementami rower (bez wskaznika 'next')
void swap_user( User*x, User*y ); //    --||--
Parking* Pointer_To_Parking ( Parking*head ); //funkcja pozwalajaca zwrocic wskaznik do jednego z miejsc parkingowych
Parking* Add_Parking_Place ( Parking*head );//dodawanie miejsca parkingowego
Parking* Remove_Parking_Place ( Parking* head, Bike*glowa ); //usuwanie elementu listy Parkingowej
Bike* Remove_Bike ( Bike*glowa, int x ); //usuwanie elementu listy rower
User* Remove_User ( User*first );//usuwanie uzytkownika
void Show_Parking_Places ( Parking*head );//wypisuje miejsca
void Show_Users ( User*first );// wypisuje uzytkownikow
void Show_Bikes( Bike*glowa );//wypisuje rowery
int Load_Nr ( void ); //scanf dla nr roweru
int Load_Int( void ); //scanf dla Int'ow
Time* Load_DateofInspection( void ); //zwraca wskaznik na data ostatniego przegladu
void Next_Inspection ( Bike*glowa ); //funkcja sprawdzajaca kiedy nastepny przeglad danego roweru
void Check_For_Incoming_Inspections( Bike*glowa );//funkcja wypisze wszytkie rowery ktorych przeglad zbliza sie
List* Make_List ( int x, Bike*glowa );//tworzenie pomocniczej listy (uzywane w funkcji rent_bike)

//enum dzielnica
enum District {BEMOWO = 1, WOLA, SRODMIESCIE, MOKOTOW, PRAGA};

//struktura rower
struct Bike{
    int nr; //nr seryjny
    Time* date; //data ostatniego przegladu
    Bike* next; //nastepny element listy
    bool status;

    Parking* parking;
};

//struktura uzytkownikow
struct User{
    int id;
    int cash;
    User* next;

    Bike* taken_bike;
    Time* date_take;
};

//struktura miejsc postojowych
struct Parking_Place{
    District dis;
    char adress[MAX_ADRESS_LENGTH];
    unsigned short int AmountOfBikes;
    Parking* next;
};

//pomocnicza struktura
struct List_Of_Parking_Places{
    Bike* bike;
    List* next;
};

void save( Parking*head, Bike*glowa, User*first ){
    FILE* file;
    Bike*glowaRecovery = glowa;
    file = fopen( "SaveParking.txt", "wb" );
    while( head ){
        fwrite( head, sizeof( Parking ), 1, file );
        while( glowa ){
            if( glowa->parking == head ){
                fwrite( &(glowa->nr), sizeof(int), 1, file );
            }
            glowa = glowa->next;
        }
        glowa = glowaRecovery;
        head = head->next;
    }
    fclose( file );

    file = fopen( "SaveBike.txt", "wb" );
    while( glowa ){
        fwrite( glowa, sizeof(Bike), 1, file );
        fwrite( glowa->date, sizeof(Time), 1, file );
        glowa = glowa->next;
    }
    fclose( file );

    file = fopen( "SaveUser.txt", "wb" );
    while( first ){
        fwrite( first, sizeof(User), 1, file );
        if( first->date_take != NULL ){
            fwrite( first->date_take, sizeof(Time), 1, file );
            fwrite( &(first->taken_bike->nr), sizeof(int), 1, file );
        }
        first = first->next;
    }
    fclose( file );
}

Parking* loadParking(void){
    FILE* file;
    file = fopen( "SaveParking.txt", "rb" );
    char c = fgetc( file );
    if( file == NULL || c == EOF ){
        return NULL;
    }
    rewind( file );

    Parking*start;
    Parking*head = malloc( sizeof(Parking) );
    Parking*nowy = malloc( sizeof(Parking) );
    //pobranie informacji o glowie
    fread( head, sizeof(Parking), 1, file );
    for( int i=0; i<head->AmountOfBikes; ++i ){

        fseek( file, sizeof(int), SEEK_CUR );
    }
    start = head;

    //pobranie informacji o reszcie
    while( fread( nowy, sizeof(Parking), 1, file )){
        for( int i=0; i<nowy->AmountOfBikes; ++i ){
            fseek( file, sizeof(int), SEEK_CUR );
        }
        head->next = nowy;
        head = nowy;
        nowy = malloc( sizeof(Parking) );
    }
    fclose( file );
    free( nowy );
    return start;
}

Bike* loadBike( Parking*head ){
    FILE* file;
    FILE* fileParking;
    Parking*headRecovery = head;
    file = fopen( "SaveBike.txt", "rb" );
    fileParking = fopen( "SaveParking.txt", "rb" );
    char c = fgetc( file );
    if( file == NULL || c == EOF ){
        return NULL;
    }
    rewind( file );

    Bike*start;
    Bike*glowa = malloc( sizeof(Bike) );
    Time*data = malloc( sizeof(Time) );
    fread( glowa, sizeof(Bike), 1, file );
    fread( data, sizeof(Time), 1, file );
    glowa->date = data;

    if( glowa->status == false ){
        bool t = true;
        int x = -1;
        while( t ){
            fseek( fileParking, sizeof(Parking), SEEK_CUR );
            for( int i = 0; i<head->AmountOfBikes; ++i ){
                fread( &x, sizeof(int), 1, fileParking);
                if( x == glowa->nr ){
                    glowa->parking = head;
                    t = false;
                }
            }
            head = head->next;
        }
    }
    start = glowa;

    //wczytywanie reszty rowerow
    Bike*nowy = malloc( sizeof(Bike) );
    data = malloc( sizeof(Time) );
    while( fread( nowy, sizeof(Bike), 1, file )){
        fread( data, sizeof(Time), 1, file );
        nowy->date = data;

        head = headRecovery;
        rewind( fileParking );
        if( nowy->status == false ){
            bool t = true;
            int x = -1;
            while( t ){
                fseek( fileParking, sizeof(Parking), SEEK_CUR );
                for( int i = 0; i<head->AmountOfBikes; ++i ){
                    fread( &x, sizeof(int), 1, fileParking);
                    if( x == nowy->nr ){
                        nowy->parking = head;
                        t = false;
                    }
                }
                head = head->next;
            }
        }

        glowa->next = nowy;
        glowa = nowy;
        nowy = malloc( sizeof(Parking) );
        data = malloc( sizeof(Time) );
    }

    fclose( file );
    fclose( fileParking );
    free( nowy );
    free( data );
    return start;
}

User* loadUser(Bike*glowa){
    FILE* file;
    Bike*glowaRecovery = glowa;
    file = fopen( "SaveUser.txt", "rb" );
    char c = fgetc( file );
    if( file == NULL || c == EOF ){
        return NULL;
    }
    rewind( file );

    User*start;
    User*first = malloc( sizeof(User) );
    Time*data;
    fread( first, sizeof(User), 1, file );
    if( first->taken_bike != NULL ){
        data = malloc( sizeof(Time) );
        fread( data, sizeof(Time), 1, file );
        first->date_take = data;

        int x;
        fread( &x, sizeof(int), 1, file );
        while( x != glowa->nr ){
            glowa = glowa->next;
        }
        first->taken_bike = glowa;
    }
    start = first;
    glowa = glowaRecovery;

    //reszta uzytkownikow
    User*nowy = malloc( sizeof(User) );
    int x;
    while( fread( nowy, sizeof(User), 1, file)){
        if( nowy->taken_bike != NULL ){
            data = malloc( sizeof(Time) );
            fread( data, sizeof(Time), 1, file );
            nowy->date_take = data;

            fread( &x, sizeof(int), 1, file );
            while( x != glowa->nr ){
                glowa = glowa->next;
            }
            nowy->taken_bike = glowa;
            glowa = glowaRecovery;
        }
        first->next = nowy;
        first = nowy;
        nowy = malloc( sizeof(User) );
    }
    free( nowy );
    fclose( file );
    return start;
}

void SHELL( Parking*head, Bike*glowa, User*first ){
    int x;
    bool k = true;
    head = loadParking();
    if( head != NULL ){
        glowa = loadBike( head );
    }
    first = loadUser( glowa );


    while(k){
    char *komenda;
    komenda = calloc(50, sizeof(char));
    scanf("%49s", komenda);

    if( strcmp(komenda, "add-bike") == 0 ) glowa = Add_Bike( glowa, head );
    else if( strcmp(komenda, "add-parking") == 0 ) head = Add_Parking_Place( head );
    else if( strcmp(komenda, "add-user") == 0 ) first = Add_User( first );
    else if( strcmp(komenda, "bike-rent") == 0 ) Rent_Bike( first,glowa,head );
    else if( strcmp(komenda, "add-money") == 0 ) Add_Cash( first );
    else if( strcmp(komenda, "leave-bike") == 0 ) Leave_Bike( first,head );
    else if( strcmp(komenda, "show-bikes") == 0 ) Show_Bikes( glowa );
    else if( strcmp(komenda, "show-users") == 0 ) Show_Users( first );
    else if( strcmp(komenda, "show-parking") == 0 ) Show_Parking_Places( head );
    else if( strcmp(komenda, "remove-bike") == 0 ){
        x = Load_Nr();
        glowa = Remove_Bike( glowa, x );
    }
    else if( strcmp(komenda, "remove-user") == 0 ) first = Remove_User( first );
    else if( strcmp(komenda, "remove-parking") == 0 ) head = Remove_Parking_Place( head, glowa );
    else if( strcmp(komenda, "next-inspection") == 0 ) Next_Inspection( glowa );
    else if( strcmp(komenda, "check-inspections") == 0 ) Check_For_Incoming_Inspections( glowa );
    else if( strcmp(komenda, "help") == 0 ) puts("Komendy (skladnia):\n"
                                                 "add-bike () - dodaje rower\n"
                                                 "add-parking (postepuj zgodnie z funkcja) - dodaje miejsce parkingowe\n"
                                                 "add-user (postepuj zgodnie z funkcja) - dodaje uzytkownika\n"
                                                 "bike-rent (id uzytkownika, dzielnica(1-5), 1) - wypozycza uzytkownikowi rower\n"
                                                 "add-money (id uzytkownika, kwota) - dodaje pieniadze na konto uzytkownika\n"
                                                 "leave-bike (postepuj zgodnie z funkcja) - zwraca rower\n"
                                                 "show-bikes/users/parking () - wypisuje liste rowerow/uzytkownikow/parkingow\n"
                                                 "remove-bike (nr roweru do usuniecia) - usuwa rower\n"
                                                 "remove-user (id uzytkownika) - usuwa uzytkownika\n"
                                                 "remove-parking (postepuj zgodnie z funkcja) - usuwa miejsce parkingowe\n"
                                                 "next-inspection (nr roweru) - pokaze date nastepnego przegladu danego roweru\n"
                                                 "check-inspections () - wypisze wszystkie rowery ktorych daty przeglady zblizaja sie\n"
                                                 "X - wyjscie z programu\n"
                                                 "cl - wyczyszczenie okna\n");
    else if( strcmp(komenda, "x") == 0 || strcmp(komenda, "X") == 0 ) k = false;
    else if( strcmp(komenda, "cl") == 0) system("cls");
    else puts("nieznana komenda, aby uzyskac pomoc wpisz help");
    free( komenda );
    }
    save( head, glowa, first );
    zwolnij( glowa, head, first );
}

void Rent_Bike ( User*first, Bike*glowa, Parking*head ){
    puts("");

    //wybranie uzytkownika
    puts("Aby wypozyczyc rower, podaj swoj identyfikator");
    int identyfikator = Load_Int();
        while( (first != NULL) && (first->id != identyfikator) ){
             first = first->next;
        }
        if( first == NULL ){
            puts("podales zly identyfikator, sprawdz liste uzytkownikow i sprobuj ponownie");
            return;
        }

    printf("Zatwierdzono id: %d\n", first->id );

    if( first->taken_bike != NULL ){
        puts( "Juz wypozyczyles rower!" );
        return;
    }

    //wybranie miejsca parkingowego i roweru
    puts("Jaka dzielnica Warszawy Cie interesuje?\n1.Bemowo 2.Wola  3.Srodmiescie  4.Mokotow  5.Praga");
    int x;
    bool t = true;
    while(t){
        t = false;
        x = Load_Int();
        if ( (x < 1) || (x > 5) ){
            t = true;
            puts("Podaj liczbe 1-5");
        }
    }

    List*list = NULL;
    list = Make_List( x, glowa );
    if( list == NULL ){
        puts("Nie ma tam jeszcze miejsc parkingowych");
        return;
    }
    List*poczatek = list;
    t = true;
    int choice;
    while(t){
        puts("Rowery na Bemowie i adresy do nich: ");
        for (int i = 1; list != NULL ; ++i){
            printf("%d. adres: %s\n", i, list->bike->parking->adress );
            list = list->next;
        }
        list = poczatek;
        puts("Wybierz ktory Cie interesuje (podaj cyfre)");
        choice = Load_Int();
        for(int i = 1; (i<choice) && (list != NULL); ++i){
            list = list->next;
        }
        if( list == NULL ){
            puts("Podales zla liczbe");
            list = poczatek;
        }
        else{
            t = false;
        }
    }

    //przydzielenie wskaznika do roweru i do daty wypozyczenia
    first->taken_bike = list->bike;
    list->bike->status = true;
    --(list->bike->parking->AmountOfBikes);
    list->bike->parking = NULL;
    printf( "wypozyczono rower o numerze %d\n", first->taken_bike->nr );

    time_t czas;
    time(&czas);
    Time*date_of_rent = malloc( sizeof(Time) );
    date_of_rent = localtime(&czas);
    first->date_take = date_of_rent;

    //zwolnienie pomocniczej listy
    List*zwolnij = NULL;
    while( list ){
        zwolnij = list;
        list = list->next;
        free ( zwolnij );
    }
}

void Add_Cash ( User*first ){
    puts("");
    puts("Aby dodac pieniadze do konta, podaj swoj identyfikator");
    int identyfikator = Load_Int();
    while( (first != NULL) && (first->id != identyfikator) ){
             first = first->next;
        }
        if( first == NULL ){
            puts("podales zly identyfikator, sprawdz liste uzytkownikow i sprobuj ponownie");
            return;
        }
    printf("Zatwierdzono id: %d\n", first->id );

    bool t = true;
    int money;
    while(t){
        t = false;
        puts("Jaka ilosc pieniedzy chcesz przelac na konto?");
        money = Load_Int();
        if( money < 0 ) {
            puts("podales ujemna wartosc");
            t = true;
        }
    }
    first->cash = money;
    printf("Transakcja przebiegla pomyslnie, dodano do konta %dzl\n", first->cash );
}

void Leave_Bike ( User*first, Parking*head ){
    puts("");
    puts("Oddajesz rower, podaj swoj identyfikator");
    int identyfikator = Load_Int();
    while( (first != NULL) && (first->id != identyfikator) ){
             first = first->next;
        }
        if( first == NULL ){
            puts("podales zly identyfikator, sprawdz liste uzytkownikow i sprobuj ponownie");
            return;
        }
    printf("Zatwierdzono id: %d\n", first->id );
    if( first->taken_bike == NULL ){
        puts("Nie wypozyczyles roweru, jak chcesz go oddac?");
        return;
    }

    puts("Na jakie miejsce parkingowe go odstawiasz?");
    bool a = true;
    int x;
    Parking*emergency = head;
    while(a){
        Show_Parking_Places( head );
        head = emergency;
        x = Load_Int();
        for( int i=1; (i<x) && (head != NULL); ++i ){
            head = head->next;
        }

        if( head == NULL ){
            puts("podales zla liczbe");
        }
        else{
            a = false;
        }
    }
    first->taken_bike->parking = head;
    first->taken_bike->status = false;

    int roznica_czasu;
    time_t teraz;
    time(&teraz);
    roznica_czasu = difftime( mktime(first->date_take), teraz );
    free(first->date_take);
    first->date_take = NULL;
    first->taken_bike = NULL;
    printf( "Zwrocono rower po %d godzinach\n", roznica_czasu / 3600 );
    printf( "Wysokosc oplaty wynosi %dzl (ilosc godzin * 10zl)\n", (roznica_czasu / 3600) * 10);
    first->cash = first->cash - ((roznica_czasu / 3600) * 10);
    printf("Aktualny stan konta: %dzl\n", first->cash);
}

User* Add_User ( User*first ){
    puts("");
    User* nowy = malloc( sizeof(User) );
    User* start = first;

    bool repeat = false;
    int i = rand() % MAX_USERS + 1;
    while(first){
        if (first->id == i){
            i = (i + 1) % MAX_USERS;
            repeat = true;
            //baza wysypie sie jezeli liczba uzytkownikow przekroczy MAX_USERS, mozna temu zapobiec zwiekszajac ta wartosc albo nie pozwalajac na dodawanie nowych uzytkownikow
        }
        first = first->next;
        if(repeat){
            first = start;
            repeat = false;
        }
    }
    nowy->id = i;

    //ulozenie listy
    first = start;
    nowy->next = first;
    start = nowy;
    bool a = true;
    while(a){
        if( (first == NULL) || (first->id > nowy->id) ){
            nowy->next = first;
            a = false;
        }
        else{
            swap_user(nowy, first);
            nowy = first;
            first = nowy->next;
        }
    }

    nowy->cash = 0;
    nowy->taken_bike = NULL;
    nowy->date_take = NULL;
    printf("ID klienta to: %d\n", nowy->id);
    puts("Pomyslnie dodano uzytkownika");
    puts("Aby dodac fundusze do utworzonego konta uzyj komendy \"add-money\"");
    return start;
}

Bike* Add_Bike ( Bike*glowa, Parking*head ){
    puts("");
    if( head == NULL ){
        puts("Najpierw musisz dodac parking");
        puts("");
        return glowa;
    }
    Bike*nowy = malloc( sizeof(Bike) );

    //wypelnianie elementu listy
    nowy->nr = Load_Nr();
    nowy->date = Load_DateofInspection();
    nowy->next = glowa;
    nowy->parking = Pointer_To_Parking( head );
    nowy->status = false;
    printf("Adres parkingu ktory wybrales to: %s\n", nowy->parking->adress);
    ++(nowy->parking->AmountOfBikes);

    //ulozenie elementu w liscie
    Bike*start = nowy; //zapamietanie poczatkowego polozenia elementu 'nowy'

    bool x = true;
        while(x){
            if( glowa == NULL ){
                nowy->next = glowa;
                x = false;
            }
            else if( (nowy->nr) == (glowa->nr) ){
                puts("Rower o takim numerze juz istnieje!");
                if ( nowy == start ){
                    Remove_Bike( start, nowy->nr );
                    return glowa;
                }
                else{
                    Remove_Bike( start, nowy->nr );
                    return start;
                }

            }

            else if( (nowy->nr) < (glowa->nr) ){
                x = false;
                nowy->next = glowa;
            }

            else{
                swap_bike(nowy, glowa);
                nowy = glowa;
                glowa = nowy->next;
            }
        }

    puts("pomyslnie dodano nowy rower");
    return start;
}

int main ( void ){
    srand( time( NULL ) );

    Parking*head = NULL;
    Bike*glowa = NULL;
    User*first = NULL;
    SHELL( head, glowa, first );

    return 0;
}

void Show_Bikes ( Bike*glowa ){
    puts("");
    char status[4];
    for (int i = 1; glowa != NULL ; ++i){
        if( glowa->status == false ) strcpy( status, "nie" );
        else strcpy( status, "tak" );
        printf("%d. Rower o numerze seryjnym: %d  wpozyczony? - %s\n", i, glowa->nr, status );
        glowa = glowa->next;
    }
}

void Show_Users ( User*first ){
    puts("");
    int nr_roweru = 0;
    for (int i = 1; first != NULL ; ++i){
        if ( first->taken_bike != NULL ) nr_roweru = first->taken_bike->nr;
        printf("%d. uzytkownik o id: %d, fundusze[%dzl], rower wypozyczony: %dnr\n", i, first->id, first->cash, nr_roweru );
        first = first->next;
        nr_roweru = 0;
    }
}

Parking* Add_Parking_Place ( Parking*head ){
    puts("");
    Parking*nowy = malloc( sizeof(Parking) );
    nowy->next = head;

    puts("Dodajemy miejsce postojowe, wybierz dzielnice:\n1.Bemowo 2.Wola  3.Srodmiescie  4.Mokotow  5.Praga ");
    bool t = true;
    while(t){
        t = false;
        int x = Load_Int();
        switch(x){
            case 1: nowy->dis = BEMOWO;
             break;
            case 2: nowy->dis = WOLA;
             break;
            case 3: nowy->dis = SRODMIESCIE;
             break;
            case 4: nowy->dis = MOKOTOW;
             break;
            case 5: nowy->dis = PRAGA;
             break;
            default:
                puts("Podaj liczbe 1-5");
                t = true;
                break;
        }
    }

    puts("Podaj adres miejsca postojowego (ul.nazwa_ulicy nr_budynku)");
    char str1[30];
    scanf("%s", str1);
    char str2[20];
    scanf("%s", str2);
    strcpy( nowy->adress, str1 );
    strcat( nowy->adress, " ");
    strcat( nowy->adress, str2);
    puts("Pomyslnie dodano nowe miejsce parkingowe");
    nowy->AmountOfBikes = 0;
    return nowy;
}

void Show_Parking_Places ( Parking*head ){
    puts("");
    for(int i=1; head != NULL; ++i){
        printf("%d: ", i);
        switch(head->dis){
            case 1: printf("BEMOWO %s\n", head->adress);
             break;
            case 2: printf("WOLA %s\n", head->adress);
             break;
            case 3: printf("SRODMIESCIE %s\n", head->adress);
             break;
            case 4: printf("MOKOTOW %s\n", head->adress);
             break;
            case 5: printf("PRAGA %s\n", head->adress);
             break;
        }
        head = head->next;
    }
}

Parking* Pointer_To_Parking ( Parking*head ){
    Parking*emergency = head;
    Show_Parking_Places(head);

    bool a = true;
    while(a){
        head = emergency;
        puts("ktory parking wybierasz?");
        int x = Load_Int();
            for( int i=1; (i<x) && (head != NULL); ++i ){
                head = head->next;
            }

            if( head == NULL ){
                puts("podales zla liczbe");
            }
            else{
                a = false;
            }
    }
    return head;
}

Parking* Remove_Parking_Place ( Parking* head, Bike* glowa ){
    puts("");
    Parking*start = head;
    Parking*previous = NULL;
    Show_Parking_Places(head);

    if( head == NULL ){
        puts("Jeszcze nie dodano miejsc parkingowyc");
        puts("");
        return head;
    }

    bool a = true;
    bool b = true;
    int x;
    while(a){
        previous = NULL;
        head = start;

        puts("ktory parking chcesz usunac?");
        while(b){
            b = false;
            x = Load_Int();
            if( x < 1 ){
                puts("Podano zla liczbe");
                b = true;
            }
        }
            for( int i=1; (i<x) && (head != NULL); ++i ){
                previous = head;
                head = head->next;
            }

            if( head == NULL ){
                puts("podales zla liczbe");
            }
            else{
                a = false;
            }
    }
    while(glowa){
       if( glowa->parking == head ){
        puts( "Na parkingu znajduje sie rowery, aby usunac miejsce parkingowe musisz je pierw wypozyczyc/usunac" );
        return start;
       }
       else{
        glowa = glowa->next;
       }
    }

    if( previous != NULL ){
        previous->next = head->next;
        free(head);
        puts("pomyslnie usunieto miejsce parkingowe");
        return start;
    }
    else{
        start = head->next;
        free(head);
        puts("pomyslnie usunieto miejsce parkingowe");
        return start;
    }
}

User* Remove_User ( User*first ){
    puts("");
    puts("Usuwanie uzytkownika, podaj jego identyfikator");
    User*previous = NULL;
    User*start = first;
    int identyfikator = Load_Int();
    while( (first != NULL) && (first->id != identyfikator) ){
             previous = first;
             first = first->next;
        }
        if( first == NULL ){
            puts("podales zly identyfikator, sprawdz liste uzytkownikow i sprobuj ponownie");
            return start;
        }
    printf("Zatwierdzono do usuniecia id: %d\n", first->id );
    if(first == start ){
        previous = first->next;
    }
    else previous->next = first->next;
    free(first);
    puts("pomyslnie usunieto uzytkownika");
    return previous;
}

Bike* Remove_Bike ( Bike*glowa, int nr ){
    puts("");
    Bike*previous = NULL;
    Bike*start = glowa;

    while( glowa !=  NULL ){
        if( glowa->nr == nr ){
            if ( previous == NULL ){
                previous = glowa->next;
                free( glowa );
                puts("pomyslnie usunieto rower");
                return previous;
            }

            else {
            previous->next = glowa->next;
            free( glowa );
            puts("pomyslnie usunieto rower");
            return start;
            }
        }

        else{
            previous = glowa;
            glowa = glowa->next;
        }
    }
    puts("Nie znaleziono roweru o takim numerze");
    return start;
}

void zwolnij( Bike* glowa, Parking* head, User* first ){
    Bike* poprzedni = NULL;
    Parking* last = NULL;
    User* ostatni = NULL;

    while( glowa ){
        poprzedni = glowa;
        glowa = glowa->next;
        free ( poprzedni->date );
        free ( poprzedni );
    }
    puts("zwolniono");
    while( head ){
        last = head;
        head = head->next;
        free ( last );
    }
    puts("zwolniono");
    while( first ){
        ostatni = first;
        first = first->next;
        if ( ostatni->date_take != NULL) free( ostatni->date_take );
        free ( ostatni );
    }
}

void swap_bike( Bike*x, Bike*y ){
    int z;
    z = x->nr;
    x->nr = y->nr;
    y->nr = z;

    Time*t;
    t = x->date;
    x->date = y->date;
    y->date = t;

    Parking*w;
    w = x->parking;
    x->parking = y->parking;
    y->parking = w;
}

void swap_user( User*x, User*y ){
    int z;
    z = x->id;
    x->id = y->id;
    y->id = z;

    z = x->cash;
    x->cash = y->cash;
    y->cash = z;

    Bike*t;
    t = x->taken_bike;
    x->taken_bike = y->taken_bike;
    y->taken_bike = t;

    Time*w;
    w = x->date_take;
    x->date_take = y->date_take;
    y->date_take = w;
}

void clean( void ){
    char c;
    do{
        c = getchar();
    }while ( (c != '\n') && (c != EOF) );
}

int Load_Nr ( void ){

    int x;
    int a = 0;
    while(a==0){
        puts("Podaj nr roweru");
        if( 1 != scanf("%d", &x) ){
            puts("nie udalo sie wczytac, sprobuj jeszcze raz");
            --a;
        }
        else if( x <= 0 ){
            puts("Podales ujemna liczbe");
            --a;
        }
        a++;
        clean();
    }
    return x;
}

int Load_Int ( void ){

    int a = 0;
    int x;
    while(a==0){
        if( 1 != scanf("%d", &x) ){
            puts("nie udalo sie wczytac, sprobuj jeszcze raz");
            --a;
            clean();
        }
        ++a;
    }
    return x;
}

Time* Load_DateofInspection( void ){
    time_t x;
    time(&x);
    Time*now = localtime(&x);
    Time* data = malloc( sizeof(Time) );
    data->tm_sec = now->tm_sec;
    data->tm_min = now->tm_min;
    data->tm_hour = now->tm_hour;
    data->tm_isdst = now->tm_isdst;
    data->tm_wday = now->tm_wday;
    data->tm_yday = now->tm_yday;

    puts("Podaj date ostatniego przegladu (dzien,miesiac,rok)");

    int y;
    bool a = true;
    while( a ){
    y = Load_Int();
        if( (y < 1) || (y > 31) ){
            puts("Podales liczbe z poza zakresu (1-31)");
            clean();
        }
        else
            a = false;
    }
    data->tm_mday = y;

    a = true;
    while( a ){
    y = Load_Int();
        if( (y < 1) || (y > 12) ){
            puts("Podales liczbe z poza zakresu (1-12)");
            clean();
        }
        else
            a = false;
    }
    data->tm_mon = y-1;

    a = true;
    while( a ){
    y = Load_Int();
        if( y < 0 ){
            puts("Podales ujemna liczbe");
            clean();
        }
        else
            a = false;
    }
    data->tm_year = y-1900;

    return data;
}

void Next_Inspection ( Bike*glowa ){
    puts("");
    int x = Load_Nr();
    bool a = true;
    while ( a && (glowa != NULL) ){
            if( glowa->nr == x ) a = false;
            else glowa = glowa->next;
    }
    if( glowa == NULL ){
        puts("Nie ma roweru o takim numerze seryjnym.");
        return ;
    }

    time_t czas;
    time(&czas);
    printf("Data nastepnego przegladu to: %d.%d.%d\n", glowa->date->tm_mday, glowa->date->tm_mon + 1, glowa->date->tm_year + 1901);
    ++(glowa->date->tm_year);
    printf("Pozostalo %.f dni\n",  difftime( mktime(glowa->date), czas ) / 86400 );
}

void Check_For_Incoming_Inspections( Bike*glowa ) {
    puts("");
    time_t czas;
    double x;
    czas = time(NULL);

    while( glowa!=NULL ){
        x = difftime( mktime(glowa->date), czas ) / 86400;

        if( x < 30 ){
            printf("Pozostalo %.f dni do przegladu roweru nr %d\n", x, glowa->nr);
            printf("Planowany jest on na dzien %d.%d.%d", glowa->date->tm_mday, glowa->date->tm_mon + 1, glowa->date->tm_year + 1901);
        }
        glowa = glowa->next;
    }
}

List* Make_List ( int x, Bike*glowa ){
    List*poczatek = NULL;
    while( glowa ){
        if( (glowa->parking != NULL) && (glowa->parking->dis == x) && (glowa->status == false) ){
            List*nowy = malloc( sizeof(List) );
            nowy->bike = glowa;
            nowy->next = poczatek;
            poczatek = nowy;
        }
        glowa = glowa->next;
    }
    return poczatek;
}
