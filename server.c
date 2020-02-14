/*This is a web server developed in c - 
Port no is 8080 */

#include<stdio.h>
#include<sys/socket.h>	
#include<netinet/in.h>
#include<stdlib.h> 	
#include<string.h> 	
#include <sys/stat.h> 	
#include <sys/mman.h> 	
#include <fcntl.h> 	
#include <unistd.h> 	

void error1(char *msg);
void error_client(FILE *data, char *errorno, char *file, char *msg, char *detail);

int main(int argc, char *argv[]){

	int c_socket ;				//client socket
	struct sockaddr_in s_addr , c_addr;	//server address & client address
	int addrlen_s = sizeof(s_addr);		//serevr address size
	int addrlen_c = sizeof(c_addr);		//client address size
	FILE *data;          
    	char response_data[1024];     
    	char method[1024] ,uri[1024], filename[1024], version[1024], filetype[1024];  
   	
    	//create socket
	int s_socket = socket(AF_INET , SOCK_STREAM, 0);
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(8080);
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(s_socket < 0){
		error1("opening socket error");
	}

	//bind socket to a socket-address 
	int bi = bind(s_socket, (struct sockaddr *) &s_addr , sizeof(s_addr));
	if(bi<0){
		error1("Binding error");
	}
	
	//listen for incoming connections and queues connection requests upto 5
	int li = listen(s_socket, 5);
	if( li<0){
		error1("Listening error");	
	}
	
	//accepting an incoming connection on bound socket
	while(1){
	c_socket = accept(s_socket,(struct sockaddr *)&c_addr,(socklen_t*)&addrlen_c);

		if(c_socket<0)
			error1("Accepting error");	
		
		if ((data = fdopen(c_socket, "r+")) == NULL){
        		error1("ERROR on fdopen");
    		}

    		//get the HTTP request line
    		fgets(response_data, 1024, data);
    		printf("%s", response_data);
    		sscanf(response_data, "%s %s %s\n", method, uri, version);

    		//if the method is not GET method, print 405 error
    		if (strcasecmp(method, "GET") != 0) {
       	 		error_client(data, method, "405", "Method Not Allowed",
                "The requested method is not allowed in this server!");

        		fclose(data);
        		close(c_socket);
        	continue;
   		}

    		//read http headers
    		fgets(response_data, 1024, data);
    		printf("%s", response_data);

    		while(strcmp(response_data, "\r\n")) {
        		fgets(response_data, 1024, data);
        		printf("%s", response_data);
    		}

    		//parse the uri(parameters) -common gateway interface
		char cgiarguments[1024];
	
    		if (!strstr(uri, "cgi-bin")) { 
        		strcpy(cgiarguments, "");
       			strcpy(filename, ".");
        		strcat(filename, uri);
                  if (uri[strlen(uri)-1] == '/'){
              		strcat(filename, "index.html");
           	  }
    		}
    
    		//if the requested file does not exist print  404 error 
		struct stat sdata;      
    		if (stat(filename, &sdata) < 0) {
        		error_client(data, filename, "404", "Not found",
               "The requested resourse could not be found on this server!");
        		fclose(data);
        		close(c_socket);
        		continue;
    		}

    		//parse multiple files types
    		if (strstr(filename, ".html"))
        		strcpy(filetype, "text/html");
    		if (strstr(filename, ".jpeg"))
        		strcpy(filetype, "image/jpeg");
    		if (strstr(filename, ".png"))
        		strcpy(filetype, "image/png");
    		if (strstr(filename, ".txt"))
        		strcpy(filetype, "text/plain");
    		if (strstr(filename, ".pdf"))
        		strcpy(filetype, "application/pdf");	

    		// print response header 
    		fprintf(data, "HTTP/1.1 200 OK\n");
    		fprintf(data, "Server: Web Server\n");
    		fprintf(data, "Content-length: %d\n", (int)sdata.st_size);
    		fprintf(data, "Content-type: %s\n", filetype);
    		fprintf(data, "\r\n");
    		fflush(data);
    
    		int fields = open(filename, O_RDONLY);
    		char *p;	
    		p = mmap(0, sdata.st_size, PROT_READ, MAP_PRIVATE, fields, 0);
    		fwrite(p, 1, sdata.st_size, data);
    		munmap(p, sdata.st_size);

    		fclose(data);
    		close(c_socket);
	}
}

//error function that shows error in binding,creating,listening socket
void error1 (char *msg){
	error1(msg);
	exit(1);
}

//405 and 404 error
void error_client(FILE *data, char *errorno, char *file, char *msg, char *detail) {
    fprintf(data, "HTTP/1.1 %s %s\n", errorno, msg);
    fprintf(data, "Content-type: text/html\n");
    fprintf(data, "\n");
    fprintf(data, "<html><title>Server Error</title>");
    fprintf(data, "<body >\n");
    fprintf(data, "<em>The Web server</em>\n"); 	
    fprintf(data, "%s: %s\n", errorno, msg);
    fprintf(data, "<p>%s: %s\n", detail, file);
}


