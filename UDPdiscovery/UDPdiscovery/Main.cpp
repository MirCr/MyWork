
/* oltre al nome utente potremmo iserire nel messaggio inviato anche un "flag" per identificare la modalit� on line o of line. 
oppure semplicemente non invia il messaggio. In questo caso bisogna gestire la possibilit� 
di attivare o disattivare l'invio del messaggio. Vedere le specifiche per questo.  */

#include<thread> 
#include<winsock2.h>
#include<iostream>
#include <mutex>
#include <thread>
#include <queue>
#include <memory>
#include<string.h>


#pragma warning(disable:4996) // magari da modificare.

#pragma comment(lib,"ws2_32.lib") //Winsock Library.

#define SERVER "192.168.56.255"  /*ip address of udp server. funziona ma da rivedere. 
Se cambio sottorete, potrebbe cambiare. dovrei quindi ogni volta recuperare 
il mio IP e settare 255 come ultima cifra? O c'� un altro modo?*/
#define BUFLEN 512  //Max length of buffer.
#define PORT 8888   //The port on which to listen for incoming data.
#define MAXQUEUESIZE 20 //Capacit� massima del buffer condiviso tra Receiver e Consumer.


void Sender(); /*invia il suo User Name ogni secondo in broadcast per sapere chi � online.
			   se necessario potrebbe inviare una stringa con le informazioni necessarie 
			   come un flag per dire se on line o no ecc....*/
void Receiver(); /* resta in ascolto delle informazioni, riceve ip e username.*/
void Consumer();

std::mutex mut; //protegge la coda.
std::queue<std::string> data_queue; //dato condiviso.
std::condition_variable data_cond; //indica che la coda non � vuota.

int main()
{
	


	std::thread t1{ Sender };
	std::thread t2{ Receiver };
	std::thread t3{ Consumer };

	t1.join();
	t2.join();
	t3.join();

	return 0;
}



void Sender() {

	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN] = "Mirko"; /* messaggio da inviare. qui dobbiamo impostare le informazioni 
									che vogliamo inserire nel payload: username, on-line o no, foto(?).*/
	WSADATA wsa;					 

	//Initialise winsock
	std::cout << "\nInitialising Winsock..." << std::endl;

	//printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout<<"Failed. Error Code :"<< WSAGetLastError();
		exit(EXIT_FAILURE);
	}
	std::cout<<"Initialised."<<std::endl;

	//create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		std::cout << "socket() failed with error code : "<< WSAGetLastError();
		exit(EXIT_FAILURE);
	}

	//setup address structure
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);


	//start communication
	while (1)
	{
		//send the message
		if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
		{
			std::cout<<"sendto() failed with error code: "<< WSAGetLastError();
			exit(EXIT_FAILURE);
		}
		Sleep(1000);
	}

	closesocket(s);
	WSACleanup();







}


void Receiver()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	


	slen = sizeof(si_other);



	//Initialise winsock
	std::cout<< std::endl <<"Initialising Winsock...";
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "Failed. Error Code : "<< WSAGetLastError();
		exit(EXIT_FAILURE);
	}
	std::cout << "Initialised." << std::endl;

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		std::cout << "Could not create socket : "<< WSAGetLastError();
	}
	std::cout << "Socket created.\n";

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		std::cout << "Bind failed with error code : "<< WSAGetLastError();
		exit(EXIT_FAILURE);
	}
	std::cout << "Bind done" << std::endl;

	//keep listening for data 
	while (1)
	{
		std::cout << "Waiting for data...";
		fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			std::cout << "recvfrom() failed with error code : " << WSAGetLastError();
			exit(EXIT_FAILURE);
		}

		//print details of the client/peer and the data received
		std::cout << "Received packet from "<< inet_ntoa(si_other.sin_addr)<<" : "<< ntohs(si_other.sin_port)<< std::endl;
		std::cout << "Data: " << buf << std::endl;

		std::lock_guard<std::mutex> lk(mut); //Attivo il lock
		if (data_queue.size() <= MAXQUEUESIZE) {
			data_queue.push(buf);}/*Inserisco il dato nella queue se non ho raggiunto il limite*/
		data_cond.notify_one();//Notifico e risveglio il consumatore.
		//onLine.insert(inet_ntoa(si_other.sin_addr));
		

	} //solo chiudendo questa, distruggo il lock_guard e quindi richiamo l'unlock che sveglier� definitivamente il consumatore.

	closesocket(s);
	WSACleanup();
}



void Consumer()
{
	while (true)
	{
		std::unique_lock<std::mutex> lk(mut); //Lock.
		data_cond.wait(lk, []() {return !data_queue.empty(); }); //Fai la wait solo se la coda non � vuota. Protezione verso le notifiche spurie.
		std::string data = data_queue.front(); //Prendo il pi� vecchio.
		//std::cout << "processo consumatore partitoooooo!!!" << std::endl;
		data_queue.pop(); //rimuovo l'elemento appena preso.
		lk.unlock();
		std::cout<< std::endl << "il dato CONSUMATO E: ";
		printf("%s\n", data.c_str());
	}
}