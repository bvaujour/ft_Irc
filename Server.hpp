/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bvaujour <bvaujour@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 14:43:27 by bvaujour          #+#    #+#             */
/*   Updated: 2024/05/31 01:05:15 by bvaujour         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <iostream>
# include "Irc.hpp"
# include "Client.hpp"

class	Server
{
	private:
		const std::string			_password;
		std::vector<struct pollfd>	_pfds;
		std::vector<Client>			_clients;
		void						serverExec();
		void						connectClient();
		void						clearClient(Client& client);
		static bool					_signal;
		void						readData(Client& client);
	public:
		Server();
		~Server();
		Server(const int& port, const std::string password);
		Server(const Server& toCpy);
		Server&	operator=(const Server& toCpy);
		static void					signalHandler(int signum);
};