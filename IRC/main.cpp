#include "irc.hpp"

Client::Client(){fd = -1; receive_guide = 0;authenticated = false; chan_operator = 0; authentified = 0; msg_authentificator = 0;};
Client::Client(int f){fd = f; receive_guide = 0;authenticated = false; chan_operator = 0; authentified = 0; msg_authentificator= 0;};
Client::Client(const Client &other){fd = other.fd; receive_guide = other.receive_guide; authenticated = other.authenticated; chan_operator = other.chan_operator; authentified = other.authentified;  msg_authentificator = other.msg_authentificator;};
Client& Client::operator=(const Client &other){ if (this != &other){fd = other.fd; receive_guide = other.receive_guide; authenticated = other.authenticated; chan_operator = other.chan_operator; authentified = other.authentified; msg_authentificator = other.msg_authentificator;}return *this;};
int Client::getfd(){return fd;};
int Client::getReceiveGuide(){return receive_guide;};
void Client::setReceiveGuide(int n){receive_guide = n;}
void Client::setfd(int f){fd = f;};
bool Client::isAuthenticated(){return authenticated;};
void Client::setAuthenticated(bool a){authenticated = a;};
std::string Client::getNick()const{if (!nickname.empty())return *nickname.rbegin();return "";};
void Client::setNick(const std::string &nick){nickname.insert(nick);};
std::string Client::getUser() const{if (!username.empty())return *username.rbegin();return "";}
void Client::setUser(const std::string &user){username.insert(user);};
void Client::removeLastNick(){if(!nickname.empty()){nickname.erase(--nickname.end());}};
void Client::removeLastUser(){if(!username.empty()){username.erase(--username.end());}};
void Client::setOperator(const std::string &chan){operator_channels.insert(chan);};
void Client::removeOperator(const std::string &chan){operator_channels.erase(chan);};
bool Client::isOperatorOf(const std::string &chan)const{return operator_channels.count(chan) > 0;};
const std::set<std::string> &Client::getOperatorChannels()const{return operator_channels;};
void Client::joinChannel(const std::string &chan){channels.insert(chan);}
void Client::leaveChannel(const std::string &chan){channels.erase(chan);}
bool Client::isInChannel(const std::string &chan)const{return channels.find(chan) != channels.end();}
const std::set<std::string> &Client::getChannels()const{return channels;}
void Client::setChanOperator(){chan_operator = 1;};
int Client::getChanOperator(){return chan_operator;};
void Client::implementeAuthentificator(){if (authentified < 2)authentified++;};
void Client::decrementeAuthentificator(){if (authentified > 0)authentified--;};
void Client::implementeMsgAuthentif(){msg_authentificator = 1;};
void Client::decrementeMsgAuthentif(){msg_authentificator = 0;};
int Client::getAuthentified(){return authentified;};
int Client::getMsgAuthentif(){return msg_authentificator;};
Client::~Client(){};
Channel::Channel(){};
Channel::~Channel(){};
Channel::Channel(const std::string& n){name = n;inviteOnly = true;topicRestricted = true; userLimit = 2147483647;};
const std::string& Channel::getName()const{ return name;};
void Channel::addClient(int fd){clients.insert(fd);};
void Channel::removeClient(int fd){clients.erase(fd);};
const std::set<int>& Channel::getClients()const{return clients;};
int Channel::firstClientYesOrNo(std::string chan, Client &c){if (clients.empty()){std::string msg = "👑 You are the first to join " + chan + ".\n"; send(c.getfd(), msg.c_str(), msg.size(), 0);c.setOperator(chan);addOperator(c.getfd());std::string op_msg = "👑 You are the operator of " + chan + ".\n";send(c.getfd(), op_msg.c_str(), op_msg.size(), 0);return(1);}return(0);};
void Channel::setInviteOnly(bool on){inviteOnly = on;};
bool Channel::isInviteOnly()const{return inviteOnly;};
void Channel::setTopicRestricted(bool on){topicRestricted = on;};
bool Channel::isTopicRestricted()const{return topicRestricted;};
void Channel::setKey(const std::string &k){key = k;};
void Channel::removeKey(){key.clear();};
bool Channel::hasKey()const{return !key.empty();};
std::string Channel::getKey()const{return key;};
void Channel::setUserLimit(int limit){userLimit = limit;};
void Channel::removeUserLimit(){userLimit = -1;};
int Channel::getUserLimit() const{return userLimit;};
void Channel::addOperator(int fd){operators.insert(fd);};
void Channel::removeOperator(int fd){operators.erase(fd);};
bool Channel::isOperator(int fd)const{return operators.count(fd);};
void Channel::invite(int fd){invited.insert(fd);};
bool Channel::isInvited(int fd)const{return invited.count(fd);};
bool isValidPort(const char *portStr){int port;for (int i = 0; portStr[i]; ++i){if (!isdigit(portStr[i]))return (false);}port = atoi(portStr);if (port < 1024 || port > 65535)return (false);return (true);}
void Channel::setTopic(std::string s){Topic= s;};
void Channel::set_can_modify_topic(int n){can_modify_topic = n;};
std::string Channel::getTopic(){return Topic;};
int Channel::get_modify_topic(){return can_modify_topic;};
void signalHandler(int signum){std::cout << "\n⚠️  Signal " << signum << " received, shutting down server...\n";};
/////////////////////////////////////////////////////////////////////////////////////////////
// 'struct sockaddr_in' represente une ipv4 et contient :

// sin_family                                         
	// Famille d’adresses (AF_INET)
// sin_port                                           
	// Numéro de port (ex: 6667)
// sin_addr                                           
	// Adresse IP (ex: 127.0.0.1)
/////////////////////////////////////////////////////////////////////////////////////////////
void remove_client(int fd, std::map<int, Client>& clients, std::vector<pollfd>& fds)
{
    close(fd);
    clients.erase(fd);

    for (auto it = fds.begin(); it != fds.end(); ++it)
	{
        if (it->fd == fd)
		{
            fds.erase(it);
            break;
        }
    }
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
    char buffer[10000];
    int bytes;
    std::string local_port_str;
    std::string local_pass;
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
    std::map<int, Client> clients;
	std::map<std::string, Channel> channels;
	signal(SIGINT, signalHandler);
    while (1)
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
                bytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
                if (bytes <= 0)
                {
                    std::cout << "❌ Client disconnected (fd=" << fds[i].fd << ")\n";
					remove_client(fds[i].fd, clients, fds);
					i--;
					continue;
                }
				buffer[bytes] = '\0';
				Client &cli = clients[fds[i].fd];
				cli._buffer += buffer;
				size_t pos;
				while ((pos = cli._buffer.find('\n')) != std::string::npos)
				{
					std::string msg = cli._buffer.substr(0, pos);
					cli._buffer.erase(0, pos + 1);
					while (!msg.empty() && (msg.back() == '\r' || msg.back() == '\n'))
						msg.pop_back();
					if (msg.empty())
						continue;
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
								"MSG #chan <text> - send a message to a channel.💬\n"
								"PRIVMSG #user <text> - send a private message to another client.📨\n"
								"INVITE #chan #user - invite a client to the channel.🫂\n"
								"KICK #chan #user - get out the user of the channel.👺\n"
								"TOPIC #chan [text] - view or modify the topic of the channel.☂️\n"
								"MODE #chan (options : +/- i, t, k, o, l) - modify channel rules.👾\n"
								"QUIT/EXIT - disconnect.🫡\n"
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
					std::cout << "fd:" << cli.getfd() << " sent " << msg.size() << " bytes\n";
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
                        close(cli.getfd());
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
							"MSG #chan <text> - send a message to a channel.💬\n"
							"PRIVMSG #user <text> - send a private message to another client.📨\n"
							"INVITE #chan #user - invite a client to the channel.🫂\n"
							"KICK #chan #user - get out the user of the channel.👺\n"
							"TOPIC #chan [text] - view or modify the topic of the channel.☂️\n"
							"MODE #chan (options : +/- i, t, k, o, l) - modify channel rules.👾\n"
							"QUIT/EXIT - disconnect.🫡\n"
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
							cli.decrementeAuthentificator();
							cli.decrementeMsgAuthentif();
							cli.removeLastNick();
							cli.removeLastUser();
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
							cli.decrementeMsgAuthentif();
							cli.removeLastNick();
							cli.removeLastUser();
							std::string error = "🤖 Wrong username : only Alphabetic char accepted\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
						else
						{
							bool taken = false;
							for (auto &pair : clients)
							{
								Client &other = pair.second;
								if (other.getUser() == tmp)
								{
									taken = true;
									break;
								}
							}
							if (taken)
							{
								std::string error = "❌ Username '" + tmp + "' is already taken.\n";
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
					}
					else if (tmp == "JOIN")
					{
						if (cli.getAuthentified() == 2 || cli.getMsgAuthentif() == 1)
						{
							while (j < parse_msg.size() && parse_msg[j] <= ' ')
								j++;
							start = j;
							while (j < parse_msg.size() && parse_msg[j] > ' ')
								j++;
							std::string channel_name = parse_msg.substr(start, j - start);
							while (j < parse_msg.size() && parse_msg[j] <= ' ')
								j++;
							std::string key_pass = parse_msg.substr(j);
							if (channel_name.empty() || channel_name[0] != '#')
							{
								std::string error = "🤖 Correct command to join a channel : JOIN #chan [key]\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							auto it = channels.find(channel_name);
							if (it != channels.end() && it->second.isInviteOnly() == false && !cli.isOperatorOf(channel_name))
							{
								std::string error = "⛔ The operator of " + channel_name + " doesn't allow you to join\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							if (it != channels.end() && it->second.hasKey())
							{
								if (key_pass.empty())
								{
									std::string error = "🔑 This channel requires a key to join. Usage: JOIN " + channel_name + " <key>\n";
									send(cli.getfd(), error.c_str(), error.size(), 0);
									continue;
								}
								else if (key_pass != it->second.getKey())
								{
									std::string error = "❌ Wrong key for channel " + channel_name + "\n";
									send(cli.getfd(), error.c_str(), error.size(), 0);
									continue;
								}
							}
							if (it != channels.end() && channels[channel_name].getUserLimit() == channels[channel_name].getClients().size())
							{
								std::string error = "❌ User limit reach for " + channel_name + "\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							if (it == channels.end())
							{
								channels[channel_name] = Channel(channel_name);
								cli.setChanOperator();
								it = channels.find(channel_name);
							}
							if (!cli.isInChannel(channel_name))
							{
								if (channels[channel_name].firstClientYesOrNo(channel_name, cli) == 1)
								{
									cli.setOperator(channel_name);
								}
								channels[channel_name].addClient(cli.getfd());
								cli.joinChannel(channel_name);
								std::string reply = "🪢 You joined " + channel_name + "\n";
								send(cli.getfd(), reply.c_str(), reply.size(), 0);
								std::cout << "💽 Client #" << cli.getfd() << " join " << channel_name << "\n";
							}
							else
							{
								std::string error = "🤖 You are already in " + channel_name + " channel\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
							}
						}
						else
						{
							std::string error = "⚠️ You need to be authenticated first before joining a channel\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
					}
					else if (tmp == "MSG")
					{
						if (cli.getAuthentified() == 2 || cli.getMsgAuthentif() == 1)
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
							if (cli.isInChannel(identifier) == false)
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
							std::string error = "⚠️ You need to be authentified first, before MSG a channel\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
					}
					else if (tmp == "PRIVMSG")
					{
						if (cli.getAuthentified() == 2 || cli.getMsgAuthentif() == 1)
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
								std::string error = "🤖 Correct command : PRIVMSG #<username> <text>\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							if (message.empty())
							{
								std::string error = "🤖 Please provide a message text.\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							std::string targetNick = identifier.substr(1);
							bool found = false;
							for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
							{
								Client &target = it->second;
								if (target.getUser() == targetNick)
								{
									found = true;
									std::string formatted = "📨 [Private] " + cli.getUser() + ": " + message + "\n";
									send(target.getfd(), formatted.c_str(), formatted.size(), 0);
									std::cout << cli.getUser() << " send to " << targetNick << ' ' << message.size() << " bytes" << std::endl; 
									break;
								}
							}
							if (!found)
							{
								std::string error = "❌ User '" + targetNick + "' not found.\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
							}
						}
						else
						{
							std::string error = "⚠️ You need to be authentified first, before message another client\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
					}
					else if (tmp == "INVITE")
					{
						if (cli.getAuthentified() == 2 || cli.getMsgAuthentif() == 1)
						{
							while (j < parse_msg.size() && parse_msg[j] <= ' ')
								j++;
							size_t start = j;
							while (j < parse_msg.size() && parse_msg[j] > ' ')
								j++;
							std::string channel_name = parse_msg.substr(start, j - start);
							while (j < parse_msg.size() && parse_msg[j] <= ' ')
								j++;
							std::string targetUser = parse_msg.substr(j);
							if (channel_name.empty() || channel_name[0] != '#' || targetUser[0] != '#')
							{
								std::string error = "🤖 Correct command : INVITE #chan #username\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							if (targetUser.empty())
							{
								std::string error = "🤖 Please provide an username.\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							auto it = channels.find(channel_name);
							if (it != channels.end() && it->second.isInviteOnly() == false && !cli.isOperatorOf(channel_name))
							{
								std::string error = "⛔ The operator of " + channel_name + " doesn't allow you to invite\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}

							else
							{
								std::string targetNick = targetUser.substr(1);
								bool found = false;
								for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
								{
									Client &target = it->second;
									if (target.getUser() == targetNick)
									{
										if (cli.isInChannel(channel_name))
										{
											if(channels[channel_name].getUserLimit() != 2147483647 && channels[channel_name].getUserLimit() == channels[channel_name].getClients().size())
											{
												std::string error = "🤖 Size max of user already reach, you cannot use INVITE for the moment...\n";
												send(cli.getfd(), error.c_str(), error.size(), 0);
											}
											else
											{
												channels[channel_name].addClient(target.getfd());
												target.joinChannel(channel_name);
												std::cout << "💽 Client #" << target.getfd() << " join " << channel_name << "\n";
												std::string formatted = "🫂  You got invited by " + cli.getUser() + " to join " + channel_name + "\n";
												send(target.getfd(), formatted.c_str(), formatted.size(), 0);
												std::string reply = "🪢 You joined " + channel_name + "\n";
												send(target.getfd(), reply.c_str(), reply.size(), 0);
												std::cout << cli.getUser() << " invite " << targetNick << " in channel " << channel_name << std::endl;
												found = true;
												continue;
											}
										}
										else
										{
											if (!cli.isInChannel(channel_name))
											{
												std::string reply = "🦧 You are not even in the channel.\n";
												send(cli.getfd(), reply.c_str(), reply.size(), 0);
											}
											else if(channels[channel_name].getUserLimit() == channels[channel_name].getClients().size())
											{
												std::string reply = "🤖 Size max of user already reach, you cannot use INVITE for the moment...\n";
												send(cli.getfd(), reply.c_str(), reply.size(), 0);
											}
										}
										found = true;
									}
								}
								if (!found)
								{
									std::string error = "❌ User '" + targetNick + "' not found.\n";
									send(cli.getfd(), error.c_str(), error.size(), 0);
								}
							}
						}
						else
						{
							std::string error = "⚠️ You need to be authentified first, before message another client\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
					}
					else if (tmp == "KICK")
					{
						if (cli.getAuthentified() == 2 || cli.getMsgAuthentif() == 1)
						{
							while (j < parse_msg.size() && parse_msg[j] <= ' ') j++;
							size_t start = j;
							while (j < parse_msg.size() && parse_msg[j] > ' ') j++;
							std::string channel_name = parse_msg.substr(start, j - start);

							while (j < parse_msg.size() && parse_msg[j] <= ' ') j++;
							start = j;
							while (j < parse_msg.size() && parse_msg[j] > ' ') j++;
							std::string targetUser = parse_msg.substr(start, j - start);

							while (j < parse_msg.size() && parse_msg[j] <= ' ') j++;
							std::string comment = parse_msg.substr(j);
							if (channel_name.empty() || targetUser.empty() || channel_name[0] != '#' || targetUser[0] != '#')
							{
								std::string error = "🤖 Correct command : KICK #channel #username [comment]\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							std::string targetusername = targetUser.substr(1);
							auto chanIt = channels.find(channel_name);
							if (chanIt == channels.end())
							{
								std::string error = "❌ Channel " + channel_name + " does not exist.\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							Channel &chan = chanIt->second;
							if (!cli.isOperatorOf(channel_name))
							{
								std::string error = "⛔ You are not an operator of " + channel_name + "\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							Client *targetClient = nullptr;
							for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
							{
								if (it->second.getUser() == targetusername || it->second.getNick() == targetusername)
								{
									targetClient = &it->second;
									break;
								}
							}
							if (!targetClient || !targetClient->isInChannel(channel_name))
							{
								std::string error = "❌ User '" + targetusername + "' is not in " + channel_name + "\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							std::cout << targetClient->getfd() << ' ' << std::endl;
							if (channels[channel_name].isOperator(targetClient->getfd()) == false)
							{
								chan.removeClient(targetClient->getfd());
								targetClient->leaveChannel(channel_name);
								std::string kickMsg = "⚠️ You have been kicked from " + channel_name;
								if (!comment.empty())
									kickMsg += " (" + comment + ")";
								kickMsg += "\n";
								send(targetClient->getfd(), kickMsg.c_str(), kickMsg.size(), 0);
								std::string notice = targetusername + " has been kicked from " + channel_name;
								if (!comment.empty())
									notice += "ℹ️ (" + comment + ")";
								notice += "\n";
								for (int fd : chan.getClients())
								{
									if (fd != targetClient->getfd())
										send(fd, notice.c_str(), notice.size(), 0);
								}
								std::string ok = "✅ " + targetusername + " kicked from " + channel_name + "\n";
								send(cli.getfd(), ok.c_str(), ok.size(), 0);

								std::cout << cli.getUser() << " kicked " << targetusername << " from " << channel_name << std::endl;
							}
							else if (channels[channel_name].isOperator(targetClient->getfd()) == true)
							{
								std::string error = "ℹ️ You cannot kick " + targetClient->getUser() + " he's an operator.\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
							}
						}
						else
						{
							std::string error = "⚠️ You need to be authentified first, before using KICK\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
					}
					else if (tmp == "TOPIC")
					{
						if (cli.getAuthentified() == 2 || cli.getMsgAuthentif() == 1)
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
								std::string error = "🤖 Correct command : TOPIC #<channel> [text]\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							std::string targetNick = identifier.substr(1);
							if(message.empty())
							{
								if (channels[identifier].isTopicRestricted() == true || cli.getChanOperator() == true)
								{
									std::string topview = "🕯️ TOPIC : " + channels[identifier].getTopic() + "\n";
									send(cli.getfd(), topview.c_str(), topview.size(), 0);
								}
								else
								{
									std::string topview = "ℹ️ You cannot see the TOPIC of " + channels[identifier].getName() + "\n";
									send(cli.getfd(), topview.c_str(), topview.size(), 0);
								}
							}
							else
							{
								if (channels[identifier].isTopicRestricted() == false && cli.getChanOperator() == false)
								{
									std::string err = "🤖 You can't modify the Topic\n";
									send(cli.getfd(), err.c_str(), err.size(),0);
									continue;
								}
								else
								{
									channels[identifier].setTopic(message);
									for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
									{
										Client &target = it->second;
										std::string change = "ℹ️  Topic has been changed.\n";
										send(target.getfd(), change.c_str(), change.size(), 0);
									}
								}
							}
						}
					}
					else if (tmp == "MODE")
					{
						if (cli.getAuthentified() == 2 || cli.getMsgAuthentif() == 1)
						{
							while (j < parse_msg.size() && parse_msg[j] <= ' ')
								j++;
							size_t start = j;
							while (j < parse_msg.size() && parse_msg[j] > ' ')
								j++;
							std::string identifier = parse_msg.substr(start, j - start);
							while (j < parse_msg.size() && parse_msg[j] <= ' ')
								j++;
							std::string options = parse_msg.substr(j);

							if (identifier.empty() || identifier[0] != '#')
							{
								std::string error = "🤖 Correct command : MODE #channel <modes>\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							std::string channel_name = identifier.substr(1);
							if (channels.find(identifier) == channels.end())
							{
								std::string error = "❌ Channel #" + channel_name + " does not exist.\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							Channel &chan = channels[identifier];
							if (!chan.isOperator(cli.getfd()))
							{
								std::string error = "⛔ You are not an operator of #" + channel_name + ".\n";
								send(cli.getfd(), error.c_str(), error.size(), 0);
								continue;
							}
							if (options.empty())
							{
								std::string msg = "ℹ️  Need to choose a mode (i, t, k, o, l)\n";
								send(cli.getfd(), msg.c_str(), msg.size(), 0);
								continue;
							}
							std::istringstream iss(options);
							std::string token;
							while (iss >> token)
							{
								if (token[0] != '+' && token[0] != '-')
								{
									std::string error = "⚠️ Mode must start with + or - (got '" + token + "')\n";
									send(cli.getfd(), error.c_str(), error.size(), 0);
									break;
									//continue;
								}
								// bool adding = (token[0]);
								for (size_t k = 1; k < token.size(); ++k)
								{
									char mode = token[k];

									if (mode == 'i')
									{
										if (token[0] == '-')
											chan.setInviteOnly(false);
										else if(token[0] == '+')
											chan.setInviteOnly(true);
									}
									else if (mode == 't')
									{
										if (token[0] == '+')
											chan.setTopicRestricted(true);
										if (token[0] == '-')
											chan.setTopicRestricted(false);
									}
									else if (mode == 'k')
									{
										std::string param;
										if (token[0] == '+')
										{
											if (!(iss >> param))
											{
												std::string err = "⚠️ MODE +k requires a password argument.\n";
												send(cli.getfd(), err.c_str(), err.size(), 0);
												continue;
											}
											chan.setKey(param);
										}
										else if (token[0] == '-')
										{
											chan.removeKey();
										}
									}
									else if (mode == 'l')
									{
										if (token[0] == '+')
										{
											std::string limitStr;
											if (!(iss >> limitStr))
											{
												std::string err = "⚠️ MODE +l requires a numeric limit.\n";
												send(cli.getfd(), err.c_str(), err.size(), 0);
												continue;
											}
											int limit = atoi(limitStr.c_str());
											if (limit <= 0)
											{
												std::string err = "⚠️ MODE +l requires a positive number.\n";
												send(cli.getfd(), err.c_str(), err.size(), 0);
												continue;
											}
											int c = chan.getClients().size();
											if(limit < c)
											{
												std::string err = "⚠️ Cannot set limit lower than current number of users (" + std::to_string(c) + ").\n";
            									send(cli.getfd(), err.c_str(), err.size(), 0);
												continue;
											}
											else
												chan.setUserLimit(limit);
										}
										else if (token[0] == '-')
										{
											chan.removeUserLimit();
										}
									}
									else if (mode == 'o')
									{
										std::string targetName;
										if (!(iss >> targetName))
										{
											std::string err = "⚠️ MODE +o requires a target username.\n";
											send(cli.getfd(), err.c_str(), err.size(), 0);
											continue;
										}
										Client *targetClient = NULL;
										for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
										{
											if (it->second.getUser() == targetName)
											{
												targetClient = &it->second;
												break;
											}
										}
										if (!targetClient)
										{
											std::string err = "❌ User '" + targetName + "' not found.\n";
											send(cli.getfd(), err.c_str(), err.size(), 0);
											continue;
										}
										if (token[0] == '+')
										{
											chan.addOperator(targetClient->getfd());
											targetClient->setOperator(identifier);
											std::string ok = "👑 " + targetName + " is now an operator of #" + channel_name + ".\n";
											send(cli.getfd(), ok.c_str(), ok.size(), 0);
										}
										else if (token[0] == '-')
										{
											chan.removeOperator(targetClient->getfd());
											targetClient->removeOperator(identifier);
											std::string ok = "⚙️  " + targetName + " is no longer operator of #" + channel_name + ".\n";
											send(cli.getfd(), ok.c_str(), ok.size(), 0);
										}
									}
									else
									{
										std::string err = "⚠️ Unknown mode '" + std::string(1, mode) + "'.\n";
										send(cli.getfd(), err.c_str(), err.size(), 0);
									}
								}
							}
							std::string done = "✅ Channel modes updated for #" + channel_name + "\n";
							send(cli.getfd(), done.c_str(), done.size(), 0);
						}
						else
						{
							std::string error = "⚠️ You need to be authentified first before using MODE.\n";
							send(cli.getfd(), error.c_str(), error.size(), 0);
						}
					}
					else
					{
						std::string reply = "❌ Wrong command : " + msg + "\n";
						send(cli.getfd(), reply.c_str(), reply.size(), 0);
					}
					if (cli.getAuthentified() == 2)
					{
						cli.implementeAuthentificator();
						std::string authentified = "You managed to be authentified 🗿\n";
						send(cli.getfd(), authentified.c_str(), authentified.size(), 0);
						cli.decrementeAuthentificator();
						cli.implementeMsgAuthentif();	
					}
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
