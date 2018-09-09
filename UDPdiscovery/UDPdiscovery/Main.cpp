//aggiungere un set o un map per salvare gli utenti che hanno risposto. o magari
//inserirli in un set di oggetti della classe utenti. qualcosa del genere.
//usare le condition varible, per passare, al thread principale, ogni volta che si trova un nuovo 
//utente. 

//qualche derscizione sotto. ******** 

/*merging of client func with serve func in order to use just a single thread to found out 
on line users. 

the strategy provides a infinite loop starting sending a broadcast message in the subnet. 
then, the threat keep listening for answers.  

goals:

- using the same socket to send and to receive. DONE
- Setting the right upd broadcast address in every subnet. */

#include<thread> 
#include<stdio.h>
#include<winsock2.h> 


#include<set>



#pragma warning(disable:4996) // da modificare ############################## togliere

#pragma comment(lib,"ws2_32.lib") //Winsock Library


#define BROADCAST_ADD "192.168.56.255" //Broadcast address
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data


void UdpDiscovery(); // resta in ascolto di messaggi e risponde con il proprio ip al mittente.



int main()
{
	


	
	std::thread t2{ UdpDiscovery };

	
	t2.join();

	return 0;
}




void UdpDiscovery()
{
	struct sockaddr_in server, Receiving_si_other, Sending_si_other;
	int s, slen = sizeof(Receiving_si_other) , recv_len;
	char buf[BUFLEN];
	char message[BUFLEN] = "chi è on line?"; //sender
	WSADATA wsa;

	std::set<std::string> onLine;

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure RECEIVING
	memset((char *)&Receiving_si_other, 0, sizeof(Receiving_si_other));
	Receiving_si_other.sin_family = AF_INET;
	Receiving_si_other.sin_port = htons(PORT);
	server.sin_addr.s_addr = INADDR_ANY;

	//setup address structure SENDING
	memset((char *)&Sending_si_other, 0, sizeof(Sending_si_other));
	Sending_si_other.sin_family = AF_INET;
	Sending_si_other.sin_port = htons(PORT);
	Sending_si_other.sin_addr.S_un.S_addr = inet_addr(BROADCAST_ADD);

	
	//Bind
	if (bind(s, (struct sockaddr *)&Receiving_si_other, sizeof(Receiving_si_other)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		Sleep(10000);
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	


	//keep sending and listening for data
	while (1)
	{
		//send the message
		if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &Sending_si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			Sleep(10000);
			exit(EXIT_FAILURE);
		}
		else
		{
			printf("data sent \n");
		}


		printf("Waiting for data...");
		fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &Receiving_si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			Sleep(10000);
			exit(EXIT_FAILURE);
		}

		//print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(Receiving_si_other.sin_addr), ntohs(Receiving_si_other.sin_port));
		printf("Data: %s\n", buf);
		onLine.insert(inet_ntoa(Receiving_si_other.sin_addr));
		//now reply the client with the same data
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &Receiving_si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			Sleep(10000);
			exit(EXIT_FAILURE);
		}

	}

	closesocket(s);
	WSACleanup();
}


/*Socket binding is process of binding a socket to a network address within the system.
When a socket is bound the server can accept client connections. 
There are two types of socket binding, explicit and implicit socket binding..... 
http://devlib.symbian.slions.net/s3/GUID-D1BCE2D9-04B5-5C39-A50B-C5BBDAAFEFED.html. */