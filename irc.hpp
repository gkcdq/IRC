#ifndef IRC_HPP
# define IRC_HPP

# include <arpa/inet.h>
# include <atomic>
# include <csignal>
# include <cstddef>
# include <cstdlib>
# include <cstring>
# include <fcntl.h>
# include <iostream>
# include <map>
# include <netdb.h>
# include <netinet/in.h>
# include <poll.h>
# include <set>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <string>
# include <sys/socket.h>
# include <sys/types.h>
# include <unistd.h>
# include <vector>

struct		Client
{
  private:
	int		fd;
	int		receive_guide;
	bool	authenticated;
	std::string nickname;
	std::string username;
	std::string current_channel;
    int chan_operator;

  public:
	Client();
	Client(int f);
	Client(const Client &other);
	Client &operator=(const Client &other);
	~Client();
	int getfd();
	void setfd(int f);
	bool isAuthenticated();
	void setAuthenticated(bool a);
	int getReceiveGuide();
	void setReceiveGuide(int n);
	std::string getNick() const;
	void setNick(const std::string &nick);
	std::string getUser() const;
	void setUser(const std::string &user);
	std::string getChannel() const;
	void setChannel(const std::string &chan);
    void setChanOperator();
};

class Channel
{
  private:
	std::string name;
	std::set<int> clients;

  public:
	Channel();
	~Channel();
	Channel(const std::string &n);
	const std::string &getName() const;
	void addClient(int fd);
	void removeClient(int fd);
	const std::set<int> &getClients() const;
};

// ------------------Initialisation du serveur
// Parse des arguments
// Création du socket serveur
// Configuration non bloquante
// bind et listen

// ------------------Boucle principale (poll)
// Acceptation des nouveaux clients
// Lecture des messages entrants
// Assemblage des paquets pour obtenir les commandes complètes
// Traitement des commandes IRC
// Envoi des réponses appropriées

// ------------------Gestion clients
// Authentification
// Gestion du nickname et du username
// Gestion des channels et des messages
// Gestion des opérateurs

// Nettoyage
// Fermeture de toutes les connexions
// Libération des ressources

// 1️⃣ socket()        → créer la socket serveur
// 2️⃣ setsockopt()    → options (réutiliser le port)
// 3️⃣ bind()          → attacher au port choisi
// 4️⃣ listen()        → attendre des connexions
// 5️⃣ poll() / select() / epoll() → surveiller plusieurs clients
// 6️⃣ accept()        → accepter un nouveau client
// 7️⃣ recv() / send() → recevoir/envoyer les messages
// 8️⃣ close()         → fermer une socket client quand il part

#endif