#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>


int main(){

  int welcomeSocket, newSocket;
  char buffer[1024];
  struct sockaddr_in serverAddr;
  struct sockaddr_in serverStorage;
  socklen_t addr_size;

  welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
  
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(7891);
  serverAddr.sin_addr.s_addr = inet_addr("10.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  


  bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  if(listen(welcomeSocket,1)==0)
    printf("Listening\n");
  else
    printf("Error\n");

  while(1) {

  addr_size = sizeof serverStorage;
  newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
  struct sockaddr_in* cliIP = (struct sockaddr_in*)&serverStorage;
  struct in_addr ipAddr = cliIP->sin_addr;

  char str[INET_ADDRSTRLEN];

  inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
  char* ID = cliIP->sin_zero;
  char str2[8];
  inet_ntop(AF_INET, &ID, str2, 8);

  printf("\nClient IP is: %s", str);
  printf("\nClient port is: %d", serverStorage.sin_port);
  printf("\nClient padding characters are (should be blank): ");

  int i;

  for (i=0;i<8;i++){
	printf("%c", serverStorage.sin_zero[i]);
  }
  printf("\nGot a client connection from IP, port: <%s, %d> (can be used for client identification)\n", inet_ntoa(serverStorage.sin_addr), serverStorage.sin_port);

  strcpy(buffer, "Hi from server");
  send(newSocket,buffer,23,0);
  recv(newSocket, buffer, 1024, 0);

  printf("\nData received from client <%s, %d>: %s\n",inet_ntoa(serverStorage.sin_addr), serverStorage.sin_port, buffer);   
  close(newSocket);

 }
  return 0;

}
