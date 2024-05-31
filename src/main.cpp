/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bvaujour <bvaujour@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 15:12:38 by bvaujour          #+#    #+#             */
/*   Updated: 2024/05/31 17:21:47 by bvaujour         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server.hpp"
#include "../Client.hpp"

int	main(int ac, char **av)
{
	if (ac == 3)
	{
		try
		{
			signal(SIGINT, Server::signalHandler);
			signal(SIGQUIT, Server::signalHandler);
			Server server(std::stoi(av[1]), av[2]);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
	}
	return (0);
}
