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

/*#include<sys/types.h>
#include<sys/shm.h>
#include <sys/sem.h>*/


static struct baza_danych  //struktura przechowujaca dane logujacych sie klientow
{
    long type;
    int idKOLEJKI; //id kolejki prywatnej serwera i klienta do komunikacji serwer->klient (i tylko w tą stronę)
    char name[20];
    int typ;//typ asynchroniczny lub synchroniczny (0 || 1)
    int temat[10];
     int czas[10];
} baza[50];

static struct odbieranie // struktura do przesylania i odbierania komunikatow/wiadomosci
{
    long type;
    int typ;
    int temat; //tematy 0-10
    char message[200];
    int idKOLEJKI; //id kolejki prywatnej serwera i klienta do komunikacji serwer->klient (i tylko w tą stronę)
} msg;

int main()
{

static int liczba_klientow=0,idKLIENT[50],pom=0,odbior_log,odbior_wia,odbior_akt,temp;

int idREJESTRACJA = msgget(0x123,0777 | IPC_CREAT);
int idAKTUALIZACJA = msgget(0x1245,0777 | IPC_CREAT); ///// stworzenie kolejki do aktualizacji informacji o subskrypcjach


        while(strcmp(baza[liczba_klientow-1].name,"Serwer_killer"))
        {
            
            pom=0;
            odbior_akt=msgrcv(idAKTUALIZACJA, &baza[liczba_klientow] ,sizeof(baza[liczba_klientow])-sizeof(long),1,IPC_NOWAIT); //sprawdzanie subskrypcji czasowej
            odbior_log=msgrcv(idREJESTRACJA, &baza[liczba_klientow] ,sizeof(baza[liczba_klientow])-sizeof(long),1,IPC_NOWAIT); //dodanie do bazy uzytkownika
            if(odbior_log!=-1 || odbior_akt!=-1)
            {
                for(int i=0;i<liczba_klientow;i++) //sprawdzanie czy uzytkownik o podanej nazwie i id kolejki juz istanieje, jesli tak to nie dolaczamy tylko logujemy
                {
                    if(!strcmp(baza[i].name,baza[liczba_klientow].name) && baza[i].idKOLEJKI==baza[liczba_klientow].idKOLEJKI)
                    {
                        for(int j=0;j<10;j++) // logowanie i akutalizacja subskrypcji danego użytkownika
                        {
                            baza[i].temat[j]=baza[liczba_klientow].temat[j];
                        }
                         baza[i].idKOLEJKI = baza[liczba_klientow].idKOLEJKI;
                        baza[i].typ=baza[liczba_klientow].typ;
                         idKLIENT[i]=msgget(baza[liczba_klientow].idKOLEJKI,0777|IPC_CREAT);//uaktualnienie kolejki jesli to koniecznie (przy ponownym logowaniu)
                        if(odbior_log!=-1)
                            strcpy(msg.message,"\n--->>Uzytkownik juz istnieje, zalogowano pomyslnie\n");
                        else
                            strcpy(msg.message,"\n--->>Usunieto subskrypcje po uplynieciu czasu\n");
                        liczba_klientow--;
                        pom++;
                        temp=i;
                        break;
                    }
                }
                 
                if(pom==0 && strcmp(baza[liczba_klientow].name,"Serwer_killer")) // jesli klient sie LOGUJE i nie jest killerem serwera
                {
                    strcpy(msg.message,"\n--->>Rejestracja przebiegla pomyslnie!\n");
                    temp=liczba_klientow;
                    idKLIENT[liczba_klientow]=msgget(baza[liczba_klientow].idKOLEJKI,0777|IPC_CREAT);//stworzenie kolejki komunikatow serwer->klient
                }
                
                if(strcmp(baza[liczba_klientow].name,"Serwer_killer")) // wyslanie wiadomosci do serwera dotyczacej informacji dotyczacej logowania/rejestracji
                {
                    msg.type=2;
                        msgsnd(idKLIENT[temp],&msg,sizeof(msg)-sizeof(long),0);
                }
                liczba_klientow++;
            }
            
             odbior_wia=msgrcv(idREJESTRACJA, &msg ,sizeof(msg)-sizeof(long),3,IPC_NOWAIT); //odbieranie wiadomosci od klientow
            if(odbior_wia!=-1)
            {
                for(int i=0;i<liczba_klientow;i++)
                {
                        if(baza[i].temat[msg.temat]==1 && baza[i].idKOLEJKI!=msg.idKOLEJKI)
                        {
                            msg.type=4;
                            msgsnd(idKLIENT[i],&msg,sizeof(msg)-sizeof(long),0);
                        }
                }
            }
            if(!strcmp(baza[liczba_klientow-1].name,"Serwer_killer"))
            {
                printf("Unicestwiony.\n");
            }
            
        }
    




execlp("ipcrm","ipcrm", "--all=msg",NULL);  ///////////// usuwaniewszystkich kolejek komunikatow z pamieci systemu
return 0;
}
