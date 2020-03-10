#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include<sys/wait.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


static struct log
{
    long type;
    int idKOLEJKI; //id kolejki prywatnej serwera i klienta do komunikacji serwer->klient (i tylko w tą stronę)
    char name[20];
    int typ;    //typ asynchroniczny lub synchroniczny (0 || 1)
    int temat[10];
    int czas[10];
} logo;

static struct wysylanie // struktura do przesylania i odbierania komunikatow/wiadomosci
{
    long type;
    int typ;
    int temat; //tematy 0-10
    char message[200];
    int idKOLEJKI; //id kolejki prywatnej serwera i klienta do komunikacji serwer->klient (i tylko w tą stronę)
} msg,msg_rec,rec[50];

static struct odbieranie /// sluzy do wysylania i odbierania informacji miedzy procesami, w odbieraniu synchronicznym, czy uzytkownik chce odebrac wiadomosci
{
    long type;
    int wiad;
} odb;


int main(int argc, char *argv[])
{

int idREJESTRACJA = msgget(0x123,0777 | IPC_CREAT), idPRYWATNA = msgget(IPC_PRIVATE,0777|IPC_CREAT),idPRYWATNA3 = msgget(IPC_PRIVATE,0777|IPC_CREAT);
int idAKTUALIZACJA = msgget(0x1245,0777|IPC_CREAT); ///// stworzenie kolejki do aktualizacji informacji o subskrypcjach
int t=0,END_KLIENT=1,s=0;
int idKLIENTA;

//nadanie wartosci strukturze 
if(argc==4)
{
    logo.idKOLEJKI = atoi(argv[1]);
    strcpy(logo.name, argv[2]);
    logo.typ = atoi(argv[3]);
    idKLIENTA=msgget(logo.idKOLEJKI,0777|IPC_CREAT); //stworzenie kolejki serwer->klient
    printf("Jakie tematy chcesz subskrybowac (10 mozliwych tematow do wyboru indeksowanych od 0, 10 to koniec oraz jak dlugi czas ich wubskrypcji (-1 to wartosc nielimitowana))?\n" );
}
else if (argc==1)// otwarcie klienta w sposob unicestwiajacy serwer
{
    logo.idKOLEJKI = 1;
    strcpy(logo.name, "Serwer_killer");
    logo.typ = 0;
    logo.type = 1;
    printf("Unicestwiono serwer\n" );
    msgsnd(idREJESTRACJA, &logo, sizeof(logo)-sizeof(long),0); //wwyslanie danych kklienta do serwera
    return 0;
}
else //blednie podanie danych klianta
{
    printf("Wystapil blad przy podawaniu danych klienta\n" );
    return 0;
}



//nadanie wartosci poczatkowej tablicy struktury
for(int i=0;i<10;i++)
{
        logo.temat[i]=0;
        logo.czas[i]=-1;
}


while(t!=10 && strcmp(logo.name,"Serwer_killer")) // dodawanie tematow do subskrypcji
{
    printf("\nTemat subskrypcji: \n");
    scanf("%d", &t);

    if(t<10)//sprawdzanie zakresu
    {
        logo.temat[t]=1;
    }
    else if(t>10)
    {
        printf("\nNie posiadamy takiego tematu w bazie.\n");
    }
    if(t==10)
    {
        printf("\nZakonczono podawanie tamatow \n");
    }
    else
    {
        printf("\nCzas subskrypcji (-1 to nielimiotwana): \n");
        scanf("%d", &s);
        logo.czas[t]=s;
    }
}
logo.type=1;
msgsnd(idREJESTRACJA, &logo, sizeof(logo)-sizeof(long),0); //wwyslanie danych kklienta do serwera
msgrcv(idKLIENTA,&msg,sizeof(msg)-sizeof(long),2,0); //czekanie na odpowiedz serwera
printf("%s\n", msg.message);

logo.type=5;
msgsnd(idPRYWATNA3, &logo, sizeof(logo)-sizeof(long),0); //wwyslanie danych procesu timerowego
logo.type=1;

//rozpoczecie procesow do obioru i wysylki
if(fork()!=0)
{
    int pom=0;
    while(END_KLIENT)
    {
        if(atoi(argv[3])==1) ////////////////////////////// jesli odbieranie jest synchronicznie
        {
                        printf("\nJaka operacje chcesz przeprowadzic? (odebrac (1), wyslac(2), wylogowac(3))\n");
                        scanf("%d", &pom);  // skanowanie odpowiedzi
                        
                        if(pom==2) // if gdy klient chce wyslac wiadomosc    ----   bardzo przydatne  przy odbieraniu synchronicznym
                        {
                            printf("\nPodaj wiadomosc do wyslania: \n");
                            scanf(" %[^\n]%*c",msg.message);
                    
                            
                            printf("Podaj typ tematu (0-9): \n");
                            scanf("%d",&msg.temat);
                            while(msg.temat<0 || msg.temat>=10)
                            {
                                printf("BLAD! Podaj ponownie: \n");
                                scanf("%d",&msg.temat);
                            }
                            
                            printf("Podaj priorytet wiadomosci do wyslania (1-9): \n");
                            scanf("%d",&msg.typ);
                            while(msg.typ<1 || msg.typ>9)
                            {
                                printf("BLAD! Podaj ponownie: \n");
                                scanf("%d",&msg.typ);
                            }
                            msg.idKOLEJKI=logo.idKOLEJKI;
                            msg.type=3;
                            msgsnd(idREJESTRACJA, &msg, sizeof(msg)-sizeof(long),0);
                        }
                        else if(pom==1)
                        {
                            odb.type=9;
                            odb.wiad = 0;
                            msgsnd(idPRYWATNA,&odb,sizeof(odb)-sizeof(long),0); // wyslanie wiadomosci do drugieg procesu o mozliwosci odczytania wiadomosci z kolejki
                            msgrcv(idPRYWATNA, &odb ,sizeof(odb)-sizeof(long),4,0); //czekanie na odebarnie wsyztskich wiadomosci przez drugi proces
                        }
                        else if (pom==3)
                        {
                                odb.type=9;
                                odb.wiad = 1;
                                 msgsnd(idPRYWATNA,&odb,sizeof(odb)-sizeof(long),0); // wyslanie wiadomosci do drugieg procesu o mozliwosci wylogowania
                                 msgsnd(idPRYWATNA,&odb,sizeof(odb)-sizeof(long),0); // wyslanie wiadomosci do drugieg procesu o mozliwosci wylogowania
                                printf("Wylogowano pomyslnie\n");
                                END_KLIENT=0;
                                
                        }
                        else
                        {
                            printf("Podano blednie.\n");
                        }
        }
        else /////////////////////////////////////////////////// jesli odbieranie NIE jest synchroniczne
        {
            printf("\nPodaj wiadomosc do wyslania (jesli chcesz sie wylogowac wpisz 'WYLOGUJ'): \n");
            
                            scanf(" %[^\n]%*c",msg.message);
                            
                            if(!strcmp(msg.message,"WYLOGUJ"))
                               {
                                    odb.type=9;
                                    odb.wiad = 1;
                                 msgsnd(idPRYWATNA,&odb,sizeof(odb)-sizeof(long),0); // wyslanie wiadomosci do drugieg procesu o mozliwosci wylogowania
                                 msgsnd(idPRYWATNA,&odb,sizeof(odb)-sizeof(long),0); // wyslanie wiadomosci do drugieg procesu o mozliwosci wylogowania
                                    printf("Wylogowano pomyslnie\n");
                                    END_KLIENT=0;
                                    
                               }
                            
                           if(END_KLIENT==1)
                           {
                                printf("Podaj typ tematu (0-9): \n");
                                scanf("%d",&msg.temat);
                                while(msg.temat<0 || msg.temat>=10)
                                {
                                    printf("BLAD! Podaj ponownie: \n");
                                    scanf("%d",&msg.temat);
                                }
                                
                                printf("Podaj priorytet wiadomosci do wyslania (1-9): \n");
                                scanf("%d",&msg.typ);
                                while(msg.typ<1 || msg.typ>9)
                                {
                                    printf("BLAD! Podaj ponownie: \n");
                                    scanf("%d",&msg.typ);
                                }
                                msg.idKOLEJKI=logo.idKOLEJKI;
                                msg.type=3;
                                msgsnd(idREJESTRACJA, &msg, sizeof(msg)-sizeof(long),0);
                           }
        }
    }
}
else //////////////////////////////////////////////////////// drugi proces
{
    if(fork()==0) /////////////////////////////////////////////dodatkowy proces do mierzenia czasu w subskrypcji czasowej
    {
        msgrcv(idPRYWATNA3, &logo ,sizeof(logo)-sizeof(long),5,0); //odbieranie wiadomosci na temat bazy tematow i czasow
        while(END_KLIENT)
        {
             sleep(1);
            msgrcv(idPRYWATNA, &odb ,sizeof(odb)-sizeof(long),9,IPC_NOWAIT); //odbieranie wiadomosci procesu o checi zakonczenia
            if(odb.wiad==1)
               {
                   END_KLIENT=0;
               }
            else ///// jesli nie chcemy konczyc, to odliczamy czas
                {
                    for(int i=0;i<10;i++)
                    {
                        if(logo.temat[i]==1 && logo.czas[i]>0)
                        {
                            logo.czas[i]--;
                        }
                        if(logo.temat[i]==1 && logo.czas[i]==0)
                        {
                            logo.temat[i]=0;
                            logo.czas[i]=-1;
                            logo.type=1;
                            msgsnd(idAKTUALIZACJA, &logo, sizeof(logo)-sizeof(long),0); //wwyslanie danych kklienta do serwera o zmianie statusu subskrypcji
                            msgrcv(idKLIENTA,&msg_rec,sizeof(msg_rec)-sizeof(long),2,0); //czekanie na odpowiedz serwera
                            printf("%s \n",msg_rec.message);
                             printf("\nPodaj wiadomosc do wyslania (jesli chcesz sie wylogowac wpisz 'WYLOGUJ'): \n");
                            
                        }
                    }
                        
                }
        }
        
    }
    ////////////////////////////////////////////////////kod 'drugiego' procesu
    int pom,temp,pom2=0,i=0,j=0,o=0; /// zmienne pomocnicze do operowania 
  while(END_KLIENT)
    {
        if(atoi(argv[3])==0)
        { 
            temp = msgrcv(idPRYWATNA, &odb ,sizeof(odb)-sizeof(long),9,IPC_NOWAIT); //odbieranie wiadomosci procesu o checi odczytania wiadomsoci przez kienta w obieraniu asynchronicznym
            if(odb.wiad==1)
               {
                   END_KLIENT=0;
               }
            pom = msgrcv(idKLIENTA, &msg_rec ,sizeof(msg_rec)-sizeof(long),4,IPC_NOWAIT); //odbieranie wiadomosci od klientow
            if(pom!=-1)
            {
                printf("\n\n--> NOWA WIADOMOSC: %s\n",msg_rec.message);
                 printf("\nPodaj wiadomosc do wyslania (jesli chcesz sie wylogowac wpisz 'WYLOGUJ'): \n");
            }
        }
        else
        {
            pom=msgrcv(idKLIENTA, &rec[i] ,sizeof(rec[i])-sizeof(long),4,IPC_NOWAIT); //odbieranie wiadomosci od klientow
            if(pom!=-1)
            {
                printf("\n--> MASZ NOWA WIADOMOSC\n");
                printf("\nJaka operacje chcesz przeprowadzic? (odebrac (1), wyslac(2), wylogowac(3))\n");
                pom2=1;
                i++;
            }
            temp = msgrcv(idPRYWATNA, &odb ,sizeof(odb)-sizeof(long),9,IPC_NOWAIT); //odbieranie wiadomosci procesu o checi odczytania wiadomsoci przez kienta w obieraniu asynchronicznym
            if(odb.wiad==1) // sprawdzam czy klient nie chce zakonczyc swojego zadania
                {
               
                   END_KLIENT=0;
               }
            if(pom2==0 && temp !=-1 && END_KLIENT==1)
            {
                printf("\n--> Nie posiadasz zadnych nowych wiadomosci\n\n");
                odb.type=4;
                odb.wiad = 0;
                 msgsnd(idPRYWATNA,&odb,sizeof(odb)-sizeof(long),0); // wyslanie wiadomosci do drugieg procesu o mozliwosci kontunuwania interfejsu uzytkownika
            }
            else if(pom2==1 && temp !=-1 && END_KLIENT==1)
            {
                printf("\n-->  Wiadomosci otrzymane: \n");
                
               for(o=1;o<10;o++)
               {
                   j=0;
                    while(j<i)
                    {
                        if(rec[j].typ==o)
                        {
                            printf(" : %s \n",rec[j].message);
                        }
                        j++;
                    }
               }
                odb.type=4;
                odb.wiad = 0;
                 msgsnd(idPRYWATNA,&odb,sizeof(odb)-sizeof(long),0); // wyslanie wiadomosci do drugieg procesu o mozliwosci kontunuwania interfejsu uzytkownika
                 pom2=0;
                 i=0;
                 j=0;
            }
             
        }
    }  
}

return 0;
}
