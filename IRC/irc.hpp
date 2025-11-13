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
#include <sstream>

class Channel;
struct		Client
{
  private:
	int		fd;
	int		receive_guide;
	bool	authenticated;
	std::set<std::string> nickname;
	std::set<std::string> username;
	int		authentified;
	std::set<std::string> channels;
	int		chan_operator;
	//
	int		msg_authentificator;
	//
	std::set<std::string> operator_channels;

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
	//
	void setUser(const std::string &user);
	void joinChannel(const std::string &chan);
	void leaveChannel(const std::string &chan);
	bool isInChannel(const std::string &chan) const;
	const std::set<std::string> &getChannels() const;
	//
	void setChanOperator();
	int getChanOperator();
	void implementeAuthentificator();
	void decrementeAuthentificator();
	void implementeMsgAuthentif();
	void decrementeMsgAuthentif();
	int getMsgAuthentif();
	int getAuthentified();
	void removeLastNick();
	void removeLastUser();
	//
	void setOperator(const std::string &chan);
    void removeOperator(const std::string &chan);
    bool isOperatorOf(const std::string &chan) const;
    const std::set<std::string> &getOperatorChannels() const;
	std::string _buffer;
};

class Channel
{
  private:
	std::string name;
	std::string Topic;
	int can_modify_topic;
	std::set<int> clients;
	//
	bool inviteOnly;        // mode +i
    bool topicRestricted;   // mode +t
    std::string key;        // mode +k
    int userLimit;          // mode +l
    std::set<int> invited;
	std::set<int> operators;

  public:
	Channel();
	~Channel();
	Channel(const std::string &n);
	const std::string &getName() const;
	void addClient(int fd);
	void removeClient(int fd);
	const std::set<int> &getClients() const;
	int firstClientYesOrNo(std::string msg, Client &c);
	//
	void setInviteOnly(bool on);
    bool isInviteOnly() const;

    void setTopicRestricted(bool on);
    bool isTopicRestricted() const;

    void setKey(const std::string &k);
    void removeKey();
    bool hasKey() const;
    std::string getKey() const;

    void setUserLimit(int limit);
    void removeUserLimit();
    int getUserLimit() const;

    void addOperator(int fd);
    void removeOperator(int fd);
    bool isOperator(int fd) const;

    void invite(int fd);
    bool isInvited(int fd) const;

	void setTopic(std::string s);
	std::string getTopic();
	void set_can_modify_topic(int n);
	int get_modify_topic();

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