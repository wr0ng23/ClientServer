# ClientServer
First version of Client-Server application with TCP sockets on C. 
Some signals like SIGPIPE and SIGINT can be trapped with signal handlers. 
Some security stuff like entering password was implemented too, when client want to connect to server.
Client can enter coordinates of the point and radius, and then sending this to the server.
Server will be perform some calculations and then client will receive message about belonging of point to the circle. 
Server also will check correctness of coordinates and radius.
