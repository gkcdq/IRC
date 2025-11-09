#include "irc.hpp"

Client::Client(){fd = -1; receive_guide = 0;authenticated = false; chan_operator = 0; authentified = 0;};
Client::Client(int f){fd = f; receive_guide = 0;authenticated = false; chan_operator = 0; authentified = 0;};
Client::Client(const Client &other){fd = other.fd; receive_guide = other.receive_guide; authenticated = other.authenticated; chan_operator = other.chan_operator; authentified = other.authentified;};
Client& Client::operator=(const Client &other){ if (this != &other){fd = other.fd; receive_guide = other.receive_guide; authenticated = other.authenticated; chan_operator = other.chan_operator; authentified = other.authentified;}return *this;};
int Client::getfd(){return fd;};
int Client::getReceiveGuide(){return receive_guide;};
void Client::setReceiveGuide(int n){receive_guide = n;}
void Client::setfd(int f){fd = f;};
bool Client::isAuthenticated(){return authenticated;};
void Client::setAuthenticated(bool a){authenticated = a;};
std::string Client::getNick()const{return nickname;};
void Client::setNick(const std::string &nick){nickname = nick;};
std::string Client::getUser()const{return username;};
void Client::setUser(const std::string &user){username = user;};
std::string Client::getChannel()const{return current_channel;}
void Client::setChannel(const std::string &chan){current_channel = chan;};
void Client::setChanOperator(){chan_operator = 1;};
int Client::getChanOperator(){return chan_operator;};
void Client::implementeAuthentificator(){if (authentified < 2)authentified++;};
void Client::decrementeAuthentificator(){if (authentified > 0)authentified--;};
int Client::getAuthentified(){return authentified;};
Client::~Client(){};

Channel::Channel(){};
Channel::~Channel(){};
Channel::Channel(const std::string& n){name = n;};
const std::string& Channel::getName()const{ return name;};
void Channel::addClient(int fd){clients.insert(fd);};
void Channel::removeClient(int fd){clients.erase(fd);};
const std::set<int>& Channel::getClients()const{return clients;};



bool	isValidPort(const char *portStr)
{
	int	port;

	for (int i = 0; portStr[i]; ++i)
	{
		if (!isdigit(portStr[i]))
			return (false);
	}
	port = atoi(portStr);
	if (port < 1024 || port > 65535)
		return (false);
	return (true);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// 'struct sockaddr_in' represente une ipv4 et contient :

// sin_family                                         
	// Famille d’adresses (AF_INET)
// sin_port                                           
	// Numéro de port (ex: 6667)
// sin_addr                                           
	// Adresse IP (ex: 127.0.0.1)
/////////////////////////////////////////////////////////////////////////////////////////////

// int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
// if (bytes > 0)
// {
//     buffer[bytes] = '\0';
//     std::cout << "message receive : " << buffer << std::endl;
//     if (std::string(buffer) == std::string(av[2]) + "\n")
//     {
//         std::string reply = "Password OK\n";
//         send(client_fd, reply.c_str(), reply.size(), 0);
//     }
//     else
//     {
//         std::string reply = "Wrong password\n";
//         send(client_fd, reply.c_str(), reply.size(), 0);
//     }
// }
// else if (bytes == 0)
//     std::cout << "client deconnected" << std::endl;
// else
//     std::cout << "recv fail" << std::endl;

//std::atomic<bool> g_running(true);
void signalHandler(int signum)
{
    std::cout << "\n⚠️  Signal " << signum << " received, shutting down server...\n";
    //g_running = false;
}

int check_correct_character(std::string s)
{
	int i = 0;
	while (s[i])
	{
		if ((s[i] < 65 || s[i] > 90) && (s[i] < 97 || s[i] > 122))
		{
			return (1);
		}
		i++;
	}
	return (0);
}


int main(int ac, char **av)
{
    size_t p1, p2;
    int remote_sock;
    int server_fd;
    int opt;
    struct sockaddr_in adr;
    int activity;
    int client_fd;
    char buffer[1024];
    int bytes;
    std::string local_port_str;
    std::string local_pass;
    // ---------------- Vérification des arguments ----------------
    if (ac != 3 && ac != 4)
    {
        std::cout << "Usage:\n  " << av[0] << " <port> <password>\n"
                  << "  " << av[0] << " host:port_network:password_network <port> <password>" << std::endl;
        return 1;
    }
    if (ac == 3)
    {
        local_port_str = av[1];
        local_pass = av[2];
    }
    if (!isValidPort(local_port_str.c_str()))
    {
        std::cout << "Error: invalid port '" << local_port_str << "' (must be 1024-65535)" << std::endl;
        return 1;
    }
    // ---------------- Création du socket serveur ----------------
    remote_sock = -1;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&adr, 0, sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_port = htons(atoi(local_port_str.c_str()));
    adr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&adr, sizeof(adr)) < 0)
    {
        perror("bind");
        return 1;
    }
    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        return 1;
    }
    std::cout << "✅ Server listening on port " << local_port_str << std::endl;
    std::vector<pollfd> fds;
    fds.push_back({server_fd, POLLIN, 0});
    if (remote_sock >= 0)
    {
        fds.push_back({remote_sock, POLLIN, 0});
        fcntl(remote_sock, F_SETFL, O_NONBLOCK);
    }
    // ---------------- Table des clients ----------------
    std::map<int, Client> clients;
	// ---------------- Table des channels ----------------
	std::map<std::string, Channel> channels;
	// ---------------- handle all signal ----------------
	signal(SIGINT, signalHandler);
    // ---------------- Boucle principale ----------------
    while (true)
    {
        activity = poll(fds.data(), fds.size(), -1);
        if (activity < 0)
        {
            perror("poll");
            break;
        }
        for (size_t i = 0; i < fds.size(); ++i)
        {
            if (!(fds[i].revents & POLLIN))
                continue;

            // --- Nouvelle connexion ---
            if (fds[i].fd == server_fd)
            {
                client_fd = accept(server_fd, NULL, NULL);
                if (client_fd < 0)
                {
                    perror("accept");
                    continue;
                }
                fcntl(client_fd, F_SETFL, O_NONBLOCK);
                std::cout << "👤 New client connected (fd=" << client_fd << ")\n";
				std::string first_notice = "\n🔒Please enter the correct password to be autothentified by the server🔒\n\n";
				send(client_fd, first_notice.c_str(), first_notice.size(), 0);
                fds.push_back({client_fd, POLLIN, 0});
                Client c(client_fd);
                clients[client_fd] = c;
            }
            else
            {
                // --- Réception de données ---
                bytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
				std::cout << "bytes = " << bytes << std::endl;
                if (bytes <= 0)
                {
                    std::cout << "❌ Client disconnected (fd=" << fds[i].fd << ")\n";
                    close(fds[i].fd);
                    clients.erase(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    i--;
                    continue;
                }
                buffer[bytes] = '\0';
                std::string msg(buffer);
                while (!msg.empty() && (msg.back() == '\r' || msg.back() == '\n'))
                    msg.pop_back();
                if (clients.find(fds[i].fd) != clients.end())
                {
                    Client &cli = clients[fds[i].fd];
                    // --- Authentification ---
                    if (!cli.isAuthenticated())
                    {
                        if (msg == local_pass)
                        {
                            std::string reply = "✅Password OK\n";
                            send(cli.getfd(), reply.c_str(), reply.size(), 0);
                            cli.setAuthenticated(true);
                            std::cout << "✅ Client #" << cli.getfd() << " authenticated\n";
							if (cli.getReceiveGuide() == 0)
							{
								std::string guide =
								"\n📍Commands available:\n\n"
								"NICK <name> - set nickname.✨\n"
								"USER <name> - set username.🌟\n"
								"JOIN #chan - join a channel.🪢\n"
								"MSG #chan <text> - send a message.💬\n"
								"PMSG #user <text> - send a private message to another client.📨\n"
								"QUIT - disconnect.🫡\n"
								"GUIDE - refresh this guide.💡\n\n";
								send(cli.getfd(), guide.c_str(), guide.size(), 0);
								cli.setReceiveGuide(1);
							}
                        }
                        else
                        {
                            std::string reply = "❌ Wrong password\n";
                            send(cli.getfd(), reply.c_str(), reply.size(), 0);
							std::cout << "❌ Client #" << cli.getfd() << " try to authenticated\n";
                        }
                        continue;
                    }
                    // --- Client authentifié ---
                    std::cout << "#" << cli.getfd() << " sent " << msg.size() << " octet(s)\n";
					std::string parse_msg = msg;
					size_t j = 0;
					while (j < parse_msg.size() && parse_msg[j] <= ' ')
						j++;
					size_t start = j;
					while (j < parse_msg.size() && parse_msg[j] > ' ')
						j++;
					std::string tmp = parse_msg.substr(start, j - start);
                    if (tmp == "QUIT" || tmp == "EXIT")
                    {
                        std::string bye = "Goodbye!🫡\n";
                        send(cli.getfd(), bye.c_str(), bye.size(), 0);
                        shutdown(cli.getfd(), SHUT_RDWR);
                        close(cli.getfd());
                        clients.erase(cli.getfd());
                        fds.erase(fds.begin() + i);
                        i--;
                        continue;
                    }
					else if (tmp == "GUIDE")
					{
						while (j < parse_msg.size() && parse_msg[j] <= ' ')
						j++;
						start = j;
						while (j < parse_msg.size() && parse_msg[j] > ' ')
							j++;
						std::string tmp = parse_msg.substr(start, j - start);
						if (tmp.empty())
						{
							std::string guide =
							"\n📍Commands available:\n\n"
							"NICK <name> - set nickname.✨\n"
							"USER <name> - set username.🌟\n"
							"JOIN #chan - join a channel.🪢\n"
							"MSG #chan <text> - send a message.💬\n"
							"PMSG #user <text> - send a private message to another client.📨\n"
							"QUIT - disconnect.🫡\n"
							"GUIDE - refresh this guide.💡\n\n";
							send(cli.getfd(), guide.c_str(), guide.size(), 0);
						}
						else
						{
							std::string error = "🤖 Correct command for guide : GUIDE\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
					}
					else if (tmp == "NICK")
					{
						while (j < parse_msg.size() && parse_msg[j] <= ' ')
						j++;
						start = j;
						while (j < parse_msg.size() && parse_msg[j] > ' ')
							j++;
						std::string tmp = parse_msg.substr(start, j - start);
						while (j < parse_msg.size() && parse_msg[j] > ' ')
							j++;
						if (tmp.empty() || j < parse_msg.size())
						{
							std::string error = "🤖 Correct command for nickname : NICK <name>\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
						else if (check_correct_character(tmp) == 1)
						{
							if (cli.getAuthentified() == 3)
								cli.decrementeAuthentificator();
							cli.decrementeAuthentificator();
							std::string error = "🤖 Wrong nickname : only Alphabetic char accepted\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
						else
						{
							if (cli.getAuthentified() != 2)
								cli.implementeAuthentificator();
							cli.setNick(tmp);
							std::string nickname = "✨ Nickname set as : " + cli.getNick() + "\n";
							send(cli.getfd(), nickname.c_str(), nickname.size(), 0);
						}
					}
					else if (tmp == "USER")
					{
						while (j < parse_msg.size() && parse_msg[j] <= ' ')
						j++;
						start = j;
						while (j < parse_msg.size() && parse_msg[j] > ' ')
							j++;
						std::string tmp = parse_msg.substr(start, j - start);
						while (j < parse_msg.size() && parse_msg[j] > ' ')
							j++;
						if (tmp.empty() || j < parse_msg.size())
						{
							std::string error = "🤖 Correct command for username : USER <name>\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
						else if (check_correct_character(tmp) == 1)
						{
							cli.decrementeAuthentificator();
							std::string error = "🤖 Wrong username : only Alphabetic char accepted\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
						else
						{
							if (cli.getAuthentified() != 2)
								cli.implementeAuthentificator();
							cli.setUser(tmp);
							std::string Username = "🌟 Username set as : " + cli.getUser() + "\n";
							send(cli.getfd(), Username.c_str(), Username.size(), 0);
						}
					}
					else if (tmp == "JOIN")
					{
						if (cli.getAuthentified() == 2)
						{
							while (j < parse_msg.size() && parse_msg[j] <= ' ')
							j++;
							start = j;
							while (j < parse_msg.size() && parse_msg[j] > ' ')
								j++;
							std::string channel_name = parse_msg.substr(start, j - start);
							while (j < parse_msg.size() && parse_msg[j] > ' ')
								j++;
							if (channel_name.empty() || j < parse_msg.size() || channel_name[0] != '#')
							{
								std::string error = "🤖 Correct command to join a channel : JOIN #chan\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
							}
							else
							{
								if (channels.find(channel_name) == channels.end())
								{
									channels[channel_name] = Channel(channel_name);
									cli.setChanOperator();
								}
								channels[channel_name].addClient(cli.getfd());
								cli.setChannel(channel_name);
								std::string reply = "🪢 You joined " + channel_name + "\n";
								send(cli.getfd(), reply.c_str(), reply.size(), 0);
								std::cout << "💽 Client #" << cli.getfd() << " join " << channel_name << "\n";
							}
						}
						else
						{
							std::string error = "⚠️ You need to be authentified first, before joining a chan\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
					}
					else if (tmp == "MSG")
					{
						while (j < parse_msg.size() && parse_msg[j] <= ' ')
							j++;
						size_t start = j;
						while (j < parse_msg.size() && parse_msg[j] > ' ')
							j++;
						std::string identifier = parse_msg.substr(start, j - start);
						while (j < parse_msg.size() && parse_msg[j] <= ' ')
							j++;
						std::string message = parse_msg.substr(j);
						if (identifier.empty() || identifier[0] != '#')
						{
							std::string error = "🤖 Correct command : MSG #<channel> <text>\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
							continue;
						}
						if (message.empty())
						{
							std::string error = "🤖 Please provide a message text.\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
							continue;
						}
						if (channels.find(identifier) == channels.end())
						{
							std::string error = "❌ Channel " + identifier + " does not exist.\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
							continue;
						}
						if (cli.getChannel() != identifier)
						{
							std::string error = "❌ You are not in channel " + identifier + ".\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
							continue;
						}
						std::string formatted = "💬 [" + identifier + "] " + cli.getNick() + ": " + message + "\n";
						for (int member_fd : channels[identifier].getClients())
						{
							if (member_fd != cli.getfd())
							{
								send(member_fd, formatted.c_str(), formatted.size(), 0);
							}
						}
						std::cout << "📢 " << cli.getNick() << " sent message to " << identifier << ": " << message << std::endl;
					}
					else
					{
						std::string reply = "You said: " + msg + "\n";
						send(cli.getfd(), reply.c_str(), reply.size(), 0);
					}
					if (cli.getAuthentified() == 2)
					{
						cli.implementeAuthentificator();
						std::string authentified = "You managed to be authentified 🗿\n";
						send(cli.getfd(), authentified.c_str(), authentified.size(), 0);	
					}
					std::cout << cli.getAuthentified();
                }
            }
        }
    }
    close(server_fd);
    if (remote_sock >= 0)
        close(remote_sock);
    return 0;
}
// --------------- OPERATOR ---------------
// ∗ KICK - Eject a client from the channel
// ∗ INVITE - Invite a client to a channel
// ∗ TOPIC - Change or view the channel topic
// ∗ MODE - Change the channel’s mode:
// · i: Set/remove Invite-only channel
// · t: Set/remove the restrictions of the TOPIC command to channel
// operators
// · k: Set/remove the channel key (password)