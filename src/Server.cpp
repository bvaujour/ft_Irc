#include "../Server.hpp"

bool Server::_signal = false;

Server::Server()
{
	
}

Server::~Server()
{
	std::vector<Client>::iterator	it;

	it = _Clients.begin() + 1; // bot est au debut sans fd ouvert
	
	while (it != _Clients.end())
	{
		close (it->getFd());
		it++;
	}
	close (_pfds[0].fd);
	std::cout << RED << "Server destructed" << RESET << std::endl;
}

Server::Server(const Server& toCpy) : _password(toCpy._password)
{
	*this = toCpy;
}

Server&	Server::operator=(const Server& toCpy)
{
	if (this != &toCpy)
	{
		_Clients = toCpy._Clients;
		_pfds = toCpy._pfds;
		_port = toCpy._port;
	}
	return (*this);
}

Server::Server(const int& port, const std::string& password_input) : _password(password_input), _port(port) {}

const std::string& Server::getDate() const
{
	return this->_date;
}

const std::string&	Server::getPassword() const
{
	return (_password);
}

void	Server::serverInit()
{
	struct pollfd		new_poll= {};
	struct sockaddr_in	sock_addr; //structure pour l'adresse internet
	int uh = 1; // necessaire car parametre opt_value de setsockopt() = const void *

	_Clients.push_back(Bot(*this));

	sock_addr.sin_family = AF_INET; //on set le type en IPV4
	sock_addr.sin_port = htons(_port); // converti le port(int) en big endian (pour le network byte order)
	sock_addr.sin_addr.s_addr = INADDR_ANY; // IMADDR_ANY = n'importe quel adresse donc full local

	new_poll.events = POLLIN; //le poll doit lire des infos;
	new_poll.revents = 0; //0 events actuellement donc par defaut;
	new_poll.fd = socket(AF_INET, SOCK_STREAM, 0); //socket creation
	if (new_poll.fd == -1)
		throw(std::runtime_error("socket creation failed"));
	_pfds.push_back(new_poll); 
	if (setsockopt(new_poll.fd, SOL_SOCKET, SO_REUSEADDR, &uh, sizeof(uh)) == - 1) //met l'option SO_REUSEADDR sur la socket = permet de reutiliser l'addresse
		throw(std::runtime_error("set option (SO_REUSEADDR) failed"));
	if (fcntl(new_poll.fd, F_SETFL, O_NONBLOCK) == -1) //met l'option O_NONBLOCK pour faire une socket non bloquante
		throw(std::runtime_error("set option (O_NONBLOCK) failed")); //lie la socket a l'adresse reutilisable (obliger de cast pour avoir toute les infos)
	if (bind(new_poll.fd, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) == -1)
		throw(std::runtime_error("socket binding failed"));
	if (listen(new_poll.fd, SOMAXCONN) == -1) //la socket devient passive = c'est une socket serveur qui attend des connections (SOMAXCONN = max_connections autorise)
		throw(std::runtime_error("listen() failed"));
	getServerCreationTime();
}

void Server::serverExec()
{
	while (this->_signal == false)
	{
		if (poll(&_pfds[0], _pfds.size(), -1) == -1 && this->_signal == false) //-1 = bloque ici attend un event, 0 = bloque pas tourne dans le while
			throw(std::runtime_error("poll() function failed"));
		for (size_t i = 0; i < _pfds.size(); i++)
		{
			if (_pfds[i].revents && POLLIN)
			{
				if (i == 0)
					connectClient();
				else
					readData(_Clients[i]);
			}
		}
	}
}

void	Server::run()
{
	serverInit();
	serverExec();
}






void	Server::connectClient()
{
	Client				client(*this);
	struct sockaddr_in 	sock_addr;
	socklen_t 			len;
	struct pollfd		new_poll = {};

	len = sizeof(sock_addr);
	new_poll.fd = accept(_pfds[0].fd, (struct sockaddr *)&sock_addr, &len);
	if (new_poll.fd == -1)
		throw std::runtime_error("Client socket creation failed");
	if (fcntl(new_poll.fd, F_SETFL, O_NONBLOCK) == -1) //met l'option O_NONBLOCK pour faire une socket non bloquante
		throw(std::runtime_error("set option (O_NONBLOCK) failed"));
	new_poll.events = POLLIN;
	new_poll.revents = 0;
	_pfds.push_back(new_poll);
	client.setFd(new_poll.fd);
	_Clients.push_back(client);
}








int	Server::ServerRecv(int fd)
{
	char buffer[1024] = {0};
	ssize_t bytes;

	bytes = recv(fd, buffer, sizeof(buffer), 0); //MSG_WAITALL MSG_DONTWAIT MSG_PEEK MSG_TRUNC
	if (bytes <= 0)
		return (0);
	buffer[bytes] = '\0';
	_receivedBuffer = buffer;
	std::cout << CYAN << "[Server receive]" << _receivedBuffer << RESET << std::endl;
	return (1);
}

void	Server::readData(Client& client)
{
	if (!ServerRecv(client.getFd()))
		return (clearClient(client));
	client.ParseAndRespond(_receivedBuffer);
}

