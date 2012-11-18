all: hps

hps: logger.o mime_types.o reply.o request_parser.o request_handler.o connection.o server.o main.o request.o
	g++ -o hps -Wall -ansi main.o logger.o mime_types.o reply.o request_parser.o \
	 	request_handler.o connection.o server.o request.o -lboost_thread -g

main.o: main.cpp server.hpp logger.hpp 
	g++ -o main.o -Wall -ansi -c main.cpp -g
logger.o: logger.cpp logger.hpp 
	g++ -o logger.o -Wall -ansi -c logger.cpp -g
mime_types.o: mime_types.cpp mime_types.hpp
	g++ -o mime_types.o -Wall -ansi -c mime_types.cpp -g
reply.o: reply.cpp reply.hpp header.hpp
	g++ -o reply.o -Wall -ansi -c reply.cpp -g
request.o: request.cpp request.hpp
	g++ -o request.o -Wall -ansi -c request.cpp -g
request_parser.o: request_parser.cpp request_parser.hpp request.hpp
	g++ -o request_parser.o -Wall -ansi -c request_parser.cpp -g
request_handler.o: request_handler.cpp request_handler.hpp mime_types.hpp reply.hpp request.hpp
	g++ -o request_handler.o -Wall -ansi -c request_handler.cpp -g
connection.o: connection.cpp connection.hpp client_info.hpp reply.hpp request.hpp request_handler.hpp request_parser.hpp
	g++ -o connection.o -Wall -ansi -c connection.cpp -g
server.o: server.cpp server.hpp connection.hpp request_handler.hpp
	g++ -o server.o -Wall -ansi -c server.cpp -lboost_thread -g

clean: 
	rm -rf *.o
