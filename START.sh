systemctl stop server.service
g++ -o server server.cpp -ljsoncpp
chmod +x server
systemctl start server.service
systemctl status server.service