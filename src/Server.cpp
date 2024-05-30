/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bvaujour <bvaujour@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/30 23:14:13 by bvaujour          #+#    #+#             */
/*   Updated: 2024/05/31 01:09:25 by bvaujour         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server.hpp"

bool Server::_signal = false;

Server::Server()
{
	
}

Server::Server()
{
	size_t	i(0);

	while (i < _pfds.size())
		close(_pfds[i++].fd);
	
}

Server::Server(const Server& toCpy)
{
	*this = toCpy;
}

Server&	Server::operator=(const Server& toCpy)
{
	if (this != &toCpy)
	{
		
	}
	return (*this);
}

Server::Server(const int& port, const std::string password_input) : _password(password_input)
{
	struct pollfd		new_poll= {};
	struct sockaddr_in	sock_addr; //structure pour l'adresse internet
	int uh = 1; // necessaire car parametre opt_value de setsockopt() = const void *

	sock_addr.sin_family = AF_INET; //on set le type en IPV4
	sock_addr.sin_port = htons(port); // converti le port(int) en big endian (pour le network byte order)
	sock_addr.sin_addr.s_addr = INADDR_ANY; // IMADDR_ANY = n'importe quel adresse donc full local

	new_poll.fd = socket(AF_INET, SOCK_STREAM, 0); //socket creation
	if (new_poll.fd == -1)
		throw(std::runtime_error("socket creation failed"));

	if (setsockopt(new_poll.fd, SOL_SOCKET, SO_REUSEADDR, &uh, sizeof(uh)) == - 1) //met l'option SO_REUSEADDR sur la socket = permet de reutiliser l'addresse
		throw(std::runtime_error("set option (SO_REUSEADDR) failed"));
	if (fcntl(new_poll.fd, F_SETFL, O_NONBLOCK) == -1) //met l'option O_NONBLOCK pour faire une socket non bloquante
		throw(std::runtime_error("set option (O_NONBLOCK) failed"));
	if (bind(new_poll.fd, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) == -1) //lie la socket a l'adresse reutilisable (obliger de cast pour avoir toute les infos)
		throw(std::runtime_error("socket binding failed"));
	if (listen(new_poll.fd, SOMAXCONN) == -1) //la socket devient passive = c'est une socket serveur qui attend des connections (SOMAXCONN = max_connections autorise)
		throw(std::runtime_error("listen() failed"));

	new_poll.events = POLLIN; //le poll doit lire des infos;
	new_poll.revents = 0; //0 events actuellement donc par defaut;
	_pfds.push_back(new_poll);
	serverExec();
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
					readData(_clients[i - 1]);
			}
		}
	}
}

void Server::signalHandler(int signum) //static
{
	(void)signum;
	std::cout << "SIGNAL RECEIVED" << std::endl;
	Server::_signal = true;
}

void Server::connectClient()
{
	Client client;
	struct sockaddr_in sock_addr;
	socklen_t len = sizeof(sock_addr);
	struct pollfd		new_poll = {};

	client.setState(LOGIN);
	client.setChannel("#general");
	client.setFd(accept(_pfds[0].fd, (struct sockaddr *)&sock_addr, &len));
	if (client.getFd() == -1)
		throw std::runtime_error("Client socket creation failed");
	if (fcntl(client.getFd(), F_SETFL, O_NONBLOCK) == -1) //met l'option O_NONBLOCK pour faire une socket non bloquante
		throw(std::runtime_error("set option (O_NONBLOCK) failed"));
	new_poll.fd = client.getFd();
	new_poll.events = POLLIN;
	new_poll.revents = 0;
	_pfds.push_back(new_poll);
	_clients.push_back(client);
	const char *welcome_msg = ":localhost 001 user :Welcome to the IRC server\r\n";
	char buffer[1024] = {0};
	read(client.getFd(), buffer, 1024);
    std::cout << buffer << std::endl;
    send(client.getFd(), welcome_msg, strlen(welcome_msg), 0);
}

void	Server::readData(Client& client)
{
	char buffer[1024] = {0};
	ssize_t bytes;
	bytes = recv(client.getFd(), buffer, sizeof(buffer), 0); //MSG_WAITALL MSG_DONTWAIT MSG_PEEK MSG_TRUNC
	if (bytes <= 0)
		clearClient(client);
	buffer[bytes] = '\0';
	std::cout << CYAN << "Server receive: " << buffer << RESET << std::endl;
	
	size_t i(0);
	
	while (i < _clients.size())
		send(_clients[i++].getFd(), buffer, strlen(buffer), 0);
	
}

void	Server::clearClient(Client& client)
{
	std::vector<Client>::iterator			it;
	std::vector<struct pollfd>::iterator	it2;

	it2 = _pfds.begin();
	while (it2 != _pfds.end() && it2->fd != client.getFd())
		it2++;
	_pfds.erase(it2);
	it = _clients.begin();
	while (it != _clients.end() && it->getFd() != client.getFd())
		it++;
	_clients.erase(it);
}