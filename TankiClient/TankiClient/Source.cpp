#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Audio.hpp>
#include<iostream>
#include<locale>
#include<vector>
#include<Windows.h>
#include <ctime>
#include <conio.h>
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

using namespace std;
using namespace sf;

RenderWindow* window;

IpAddress serverIp = "26.81.195.38";
unsigned short serverPort = 777;

class Client
{
public:
	int sendRate = 2;
	UdpSocket dataSocket;
	TcpSocket regSocket;
	Clock sendRateTimer;
	unsigned short dataPort;
	int id = -1;
	int roomId = -1;
	void init()
	{
		Socket::Status status = dataSocket.bind(Socket::AnyPort);

		if (status != Socket::Status::Done)
		{
			while (true)
			{
				status = dataSocket.bind(Socket::AnyPort);
				if (status == Socket::Status::Done) break;
			}
		}
	}
	void close()
	{
		cout << "Отключение клиента от сервера..." << endl;
		regSocket.disconnect();
		cout << "Клиент отключен" << endl;
	}
	void sendToServerRegData(string clientName) //
	{
		if (!regSocket.isBlocking()) regSocket.setBlocking(true);
		Packet sendPacket;
		sendPacket << clientName << dataSocket.getLocalPort();
		unsigned short a;
		a = dataSocket.getLocalPort();
		cout << a << endl;
		if (regSocket.send(sendPacket) != Socket::Status::Done)
		{
			close();
		}
	}
	void receiveFromServerRegData()
	{
		if (!regSocket.isBlocking()) regSocket.setBlocking(true);
		Packet receivePacket;
		//dataSocket.receive(receivePacket, serverIp, serverPort);
		regSocket.receive(receivePacket);
		unsigned short a;
		a = dataSocket.getLocalPort();
		receivePacket >> id;
		cout << "PORT - " << a << endl;
		cout << "ID - " << id << endl;
	}
	bool registerOnServer(string clientName)
	{
		init();
		cout << "Подключение к серверу..." << endl;
		if (!regSocket.isBlocking()) regSocket.setBlocking(true);
		if (regSocket.connect(serverIp, serverPort, seconds(3)) == Socket::Status::Done)
		{
			cout << "Подключение к серверу успешно установлено" << endl;
			sendToServerRegData(clientName);
			receiveFromServerRegData();
			return true;
		}
		cout << "Ошибка подключения к серверу" << endl;
		return false;
	}
	void receivePacket(Packet& receivedPacket)
	{
		IpAddress tempIp = serverIp;
		unsigned short tempPort = serverPort;
		if (dataSocket.isBlocking()) dataSocket.setBlocking(false);
		dataSocket.receive(receivedPacket, tempIp, tempPort);
	}
	void sendPacket(Packet& sendedPacket)
	{
		IpAddress tempIp = serverIp;
		unsigned short tempPort = serverPort;
		if (dataSocket.isBlocking()) dataSocket.setBlocking(false);
		dataSocket.send(sendedPacket, tempIp, tempPort);
	}
};

Client client;
Time cycleTime;
Clock cycleTimer;
Clock sendRateTimer;
Clock attackRateTimer;
Font font;
int scene = 0;
Text Start[3];
Event event;

class Bullet
{
public:
	int id;
	Sprite bulletSprite;
	void load(Texture& texture)
	{
		bulletSprite.setTexture(texture);
		bulletSprite.setTextureRect(IntRect(0, 0, 6, 16));
		bulletSprite.setOrigin(Vector2f(8, 3));
	}
	Vector2f getPos() { return bulletSprite.getPosition(); };
};

class Player
{
public:
	Text text;
	Sprite sprite;
	Image i_player;
	Texture t_player;
	Texture t_bullet;
	Font font;
	int idPlayer = -1, roomId = -1;
	bool p = false;
	string name;
	vector<Player> vectorPlayers;
	Sprite s1, s2;
	Texture t1, t2;
	vector<Bullet> bullets;
	string messageSend;
	string messageReceive;
	bool isChatting = false;
	bool isGetRooms = false;
	float angleP = 0;
	int dir;
	int map[12][16];
	bool isFlick = false;
	Clock flickRateTimer;
	int flickerTime = 0;
	Sound soundAttack;
	SoundBuffer soundBuffer;
	void setPosition(Vector2f newPos)
	{
		sprite.setPosition(newPos);
		text.setPosition(newPos.x + sprite.getGlobalBounds().width / 2 - text.getGlobalBounds().width / 2, sprite.getPosition().y - text.getGlobalBounds().height);
	}
	void move(Vector2f normalizedMovementVec, Time cycleTime)
	{
		sprite.move({ normalizedMovementVec.x * 50 * cycleTime.asSeconds(), normalizedMovementVec.y * 50 * cycleTime.asSeconds() });
		text.move({ normalizedMovementVec.x * 50 * cycleTime.asSeconds(), normalizedMovementVec.y * 50 * cycleTime.asSeconds() });
	}
	void draw()
	{
		window->draw(text);
		window->draw(sprite);
	}
	bool isPossesed() { return p; };
	Vector2f getPos() { return sprite.getPosition(); };

	void setFlickTime(int time = 2000)
	{
		flickerTime = clock() + time;
		flickRateTimer.restart();
		isFlick = true;
	}

	void UpdateFlicker(bool isEnemy = 0)
	{
		if (flickerTime > clock())
		{
			if (flickRateTimer.getElapsedTime().asMilliseconds() > 30) {
				isFlick = !isFlick;
				flickRateTimer.restart();
			}
		}
		else {
			isFlick = false;
			flickerTime = 0;
		}

		if (isEnemy) {
			sprite.setColor(Color(255, 0, 0, (isFlick == false ? 255 : 0)));
		}
		else {
			sprite.setColor(Color(255, 255, 255, (isFlick == false ? 255 : 0)));
		}
	}

	void work()
	{
		Packet receivedPacket;
		string namePacket;
		client.receivePacket(receivedPacket);
		if (receivedPacket.getDataSize() > 0)
		{
			receivedPacket >> namePacket;
			if (namePacket == "GETROOMS")
			{
				isGetRooms = true;
				int idR, countP, countR = 0, countA = 0;
				string nameP;
				while (!receivedPacket.endOfPacket())
				{
					cout << countR << ") ";
					receivedPacket >> idR;
					cout << idR << " ";
					receivedPacket >> countP;
					for (int j = 0; j < countP; j++)
					{
						countA++;
						receivedPacket >> nameP;
						cout << nameP << " ";
					}
					cout << endl;
					countR++;
				}
				if (countR == 0) scene = 4;
			}
			if (namePacket == "NEWCLIENT")
			{
				//cout << "ПОЛУЧИЛ ПАКЕТ: " << namePacket << endl;
				string temp;
				receivedPacket >> temp;
				cout << "Новый пользователь подключился: " << temp << endl;
			}
			if (namePacket == "MESSAGE")
			{
				//cout << "ПОЛУЧИЛ ПАКЕТ: " << namePacket << endl;
				int id;
				string tempName, tempMes;
				receivedPacket >> id;
				receivedPacket >> tempName;
				receivedPacket >> tempMes;
				if (id == client.id)
				{
					HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
					SetConsoleTextAttribute(console, FOREGROUND_GREEN);
					cout << tempName << ": " << tempMes << endl;
				}
				else
				{
					HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
					SetConsoleTextAttribute(console, FOREGROUND_RED);
					cout << tempName << ": " << tempMes << endl;
				}
				HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(console, 15);
			}
			if (namePacket == "SETROOM")
			{
				//cout << "ПОЛУЧИЛ ПАКЕТ: " << namePacket << endl;
				receivedPacket >> client.roomId;
				if (client.roomId != -1)
				{
					scene = 6;
				}
				else
				{
					cout << "ERROR CONNECTROOM" << endl;
				}
				//cout << "PLAYER ID - " << client.id << endl;
				//cout << "ROOM ID - " << client.roomId << endl;
			}
			if (namePacket == "UPDATEROOM")
			{
				//cout << "ПОЛУЧИЛ ПАКЕТ: " << namePacket << endl;
				int count;
				receivedPacket >> count;
				for (int i = 0; i < count; i++)
				{
					int ID;
					float POSX, POSY;
					string NAME;
					receivedPacket >> ID;
					receivedPacket >> NAME;
					receivedPacket >> POSX;
					receivedPacket >> POSY;
					if (ID != client.id)
					{
						Player tempPlayer;
						tempPlayer.idPlayer = ID;
						tempPlayer.load(t_player, font);
						tempPlayer.sprite.setColor(Color::Red);
						tempPlayer.setPosition(Vector2f(POSX, POSY));
						vectorPlayers.push_back(tempPlayer);
					}/*
					cout << "ID: " << ID << endl;
					cout << "NAME: " << NAME << endl;
					cout << "POSX: " << POSX << endl;
					cout << "POSY: " << POSY << endl;*/
				}
			}
			if (namePacket == "SETMAP")
			{
				//cout << "ПОЛУЧИЛ ПАКЕТ: " << namePacket << endl;
				for (int i = 0; i < 12; i++)
				{
					for (int j = 0; j < 16; j++)
					{
						receivedPacket >> map[i][j];
					}
				}
				/*for (int i = 0; i < 12; i++)
				{
					for (int j = 0; j < 16; j++)
					{
						cout << map[i][j] << " ";
					}
					cout << endl;
				}*/
			}
			if (namePacket == "DATA")
			{
				int count;
				receivedPacket >> count;
				for (int i = 0; i < count; i++)
				{
					int ID;
					float POSX, POSY, ANGLE;
					receivedPacket >> ID;
					receivedPacket >> POSX;
					receivedPacket >> POSY;
					receivedPacket >> ANGLE;
					if (ID == client.id)
					{
						setPosition(Vector2f(POSX, POSY));
						sprite.setRotation(ANGLE);
					}
					for (int i = 0; i < vectorPlayers.size(); i++)
					{
						if (vectorPlayers[i].idPlayer == ID)
						{
							vectorPlayers[i].setPosition(Vector2f(POSX, POSY));
							//cout << POSX << " " << POSY << endl;
							vectorPlayers[i].sprite.setRotation(ANGLE);
						}
					}
				}
				//cout << "Принял позицию серверу: " << player.getPos().x << " " << player.getPos().y << endl;
			}
			if (namePacket == "BULLET")
			{
				while (!receivedPacket.endOfPacket())
				{
					int ID, BULLETID;
					float POSX, POSY, ANGLE;
					receivedPacket >> BULLETID;
					receivedPacket >> ANGLE;
					receivedPacket >> POSX;
					receivedPacket >> POSY;
					//cout << POSX << " " << POSY << " " << ANGLE << endl;
					int FindIndex = -1;
					for (int i = 0; i < bullets.size(); i++)
					{
						if (bullets[i].id == BULLETID)
						{
							FindIndex = i;
							break;
						}
					}
					if (FindIndex == -1)
					{
						Bullet b;
						t_bullet.loadFromFile("bullet.png");
						b.id = BULLETID;
						b.bulletSprite.setRotation(ANGLE);
						b.bulletSprite.setPosition(Vector2f(POSX, POSY));
						b.load(t_bullet);
						bullets.push_back(b);
						soundAttack.play();
					}
					else
					{
						bullets[FindIndex].bulletSprite.setPosition(Vector2f(POSX, POSY));
					}
				}
				//cout << "Принял позицию серверу: " << player.getPos().x << " " << player.getPos().y << endl;
			}
			if (namePacket == "DELETEPLAYER")
			{
				int ID;
				receivedPacket >> ID;
				if (ID == client.id)
				{
					scene = 0;
					client.roomId = -1;
					isGetRooms = false;
				}
				bullets.clear();
				vectorPlayers.clear();
			}
			if (namePacket == "DELETEBULLET")
			{
				int INDEX = -1, BULLETID;
				receivedPacket >> BULLETID;
				for (int i = 0; i < bullets.size(); i++)
				{
					if (bullets[i].id == BULLETID)
					{
						INDEX = i;
						break;
					}
				}
				if (INDEX != -1)
				{
					bullets.erase(bullets.cbegin() + INDEX, bullets.cbegin() + INDEX + 1);
				}
			}
			if (namePacket == "SETHEALTH")
			{
				int ID, hp;
				receivedPacket >> ID;
				receivedPacket >> hp;
				if (ID == client.id)
				{
					HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
					SetConsoleTextAttribute(console, FOREGROUND_GREEN);
					cout << "Ваше здоровье: " << hp << endl;
					setFlickTime();
				}
				else
				{
					HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
					SetConsoleTextAttribute(console, FOREGROUND_RED);
					cout << "Здоровье врага: " << hp << endl;
					vectorPlayers[0].setFlickTime();
				}
				HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(console, 15);
			}
			if (namePacket == "KILL701")
			{
				int ID;
				receivedPacket >> ID;
				if (ID == client.id)
				{
					HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
					SetConsoleTextAttribute(console, FOREGROUND_RED);
					cout << "ВЫ ПРОИГРАЛИ!!!" << endl;
					scene = 0;
					client.roomId = -1;
					isGetRooms = false;
				}
				else
				{
					HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
					SetConsoleTextAttribute(console, FOREGROUND_GREEN);
					cout << "ВЫ ПОБЕДИЛИ!!!" << endl;
				}
				HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(console, 15);
				bullets.clear();
				vectorPlayers.clear();
			}
		}
		switch (scene)
		{
		case 0: //Главное
		{
			window->clear();
			Start[0].setString("Список комнат");
			Start[1].setString("Выход");
			for (int i = 0; i < 3; i++)
			{
				Start[i].setFont(font);
				Start[i].setFillColor(Color::White);
				Start[i].setStyle(Text::Bold);
				Start[i].setScale(1.25, 1.25);
				Start[i].setPosition(SCREEN_WIDTH / 2 - Start[i].getGlobalBounds().width / 2, 250 * (i + 1));
			}
			while (window->pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window->close();
			}
			window->waitEvent(event);
			for (int i = 0; i < 3; i++)
			{
				if (Start[i].getGlobalBounds().contains(window->mapPixelToCoords(sf::Mouse::getPosition(*window))))
				{
					if (event.type == event.MouseButtonPressed && event.mouseButton.button == Mouse::Left && i == 0) scene = 1;
					if (event.type == event.MouseButtonPressed && event.mouseButton.button == Mouse::Left && i == 1) scene = 3;
					Start[i].setOutlineColor(Color(133, 219, 50));
					Start[i].setOutlineThickness(3);
				}
				else
				{
					Start[i].setOutlineThickness(0);
				}
			}
			for (int i = 0; i < 3; i++)
			{
				window->draw(Start[i]);
			}
			break;
		}
		case 1: //список серв
		{
			window->clear();

			Start[0].setString("Создание комнаты");
			Start[1].setString("Поиск комнаты");
			for (int i = 0; i < 2; i++)
			{
				Start[i].setFont(font);
				Start[i].setFillColor(Color::White);
				Start[i].setStyle(Text::Bold);
				Start[i].setScale(1.25, 1.25);
				Start[i].setPosition(SCREEN_WIDTH / 2 - Start[i].getGlobalBounds().width / 2, 250 * (i + 1));
			}
			window->pollEvent(event);
			while (window->pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window->close();
			}
			window->waitEvent(event);
			for (int i = 0; i < 2; i++)
			{
				if (Start[i].getGlobalBounds().contains(window->mapPixelToCoords(sf::Mouse::getPosition(*window))))
				{
					if (event.type == event.MouseButtonPressed && event.mouseButton.button == Mouse::Left) scene = i + 4;
					Start[i].setOutlineColor(Color(133, 219, 50));
					Start[i].setOutlineThickness(3);
				}
				else
				{
					Start[i].setOutlineThickness(0);
				}
			}

			for (int i = 0; i < 2; i++)
			{
				window->draw(Start[i]);
			}
			break;
		}
		case 3: //Выход
		{
			window->close();
			break;
		}
		case 4: //Создание комнаты
		{
			if (client.roomId == -1)
			{
				Packet sendedPacket;
				int selectMap = 0;
				string nameRoom = "";
				cout << "Выберите карту: ";
				cin >> selectMap;
				cout << "Введите название комнаты: ";
				cin >> nameRoom;
				sendedPacket << "CREATEROOM" << client.id << selectMap << nameRoom;
				client.sendPacket(sendedPacket);
				isGetRooms = false;
				sleep(milliseconds(2000));
				system("cls");
				cout << "Чат:" << endl;
			}
			break;
		}
		case 5:
		{
			Packet sendedPacket;
			if (client.roomId == -1 && !isGetRooms)
			{
				sendedPacket.clear();
				sendedPacket << "GETROOMS" << client.id;
				client.sendPacket(sendedPacket);
				cout << "Отправил GET: " << client.id << endl;
				sleep(milliseconds(2000));
			}
			if (client.roomId == -1 && isGetRooms)
			{
				isGetRooms = false;
				sendedPacket.clear();
				cout << "Введите номер комнаты: ";
				cin >> client.roomId;
				sendedPacket.clear();
				sendedPacket << "FINDROOM";
				sendedPacket << client.id << client.roomId;
				client.sendPacket(sendedPacket);
				sleep(milliseconds(2000));
				system("cls");
				cout << "Чат:" << endl;
			}
			break;
		}
		case 6:
		{
			cycleTime = cycleTimer.restart();
			while (window->pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					window->close();
				}
			}

			if (window->hasFocus() && client.roomId != -1)
			{
				if (!isChatting)
				{
					float dirX = 0, dirY = 0, angle = sprite.getRotation();
					if (Keyboard::isKeyPressed(Keyboard::W))
					{
						dirX = 0;
						dirY = -2;
						angle = 0;
					}
					else  if (Keyboard::isKeyPressed(Keyboard::D))
					{
						dirX = 2;
						dirY = 0;
						angle = 90;
					}
					else if (Keyboard::isKeyPressed(Keyboard::S))
					{
						dirX = 0;
						dirY = 2;
						angle = 180;
					}
					else if (Keyboard::isKeyPressed(Keyboard::A))
					{
						dirX = -2;
						dirY = 0;
						angle = 270;
					}
					if (sendRateTimer.getElapsedTime().asMilliseconds() > 7)
					{
						Packet sendedPacket;
						sendedPacket << "DATA" << client.id << dirX << dirY << angle;
						client.sendPacket(sendedPacket);
						sendRateTimer.restart();
					}
				}
				if (Keyboard::isKeyPressed(Keyboard::Escape))
				{
					scene = 7;
				}
				if (Keyboard::isKeyPressed(Keyboard::Enter))
				{
					cout << "Введите сообщение: ";
					cin >> messageSend;
					Packet sendedPacket;
					sendedPacket << "MESSAGE" << client.id << name << messageSend;
					client.sendPacket(sendedPacket);
				}
				if (Keyboard::isKeyPressed(Keyboard::Space) && attackRateTimer.getElapsedTime().asSeconds() > 2)
				{
					//bullets.push_back(Bullet);
					Packet sendedPacket;
					sendedPacket << "BULLET" << client.id << sprite.getRotation();
					client.sendPacket(sendedPacket);
					attackRateTimer.restart();
				}
				/*if (sendRateTimer.getElapsedTime().asMilliseconds() > 7)
				{
					Packet sendedPacket;
					sendedPacket << "DATA" << client.id << getPos().x << getPos().y << sprite.getRotation();
					client.sendPacket(sendedPacket);
					sendRateTimer.restart();
				}*/
			}

			window->clear();
			for (int i = 0; i < 12; i++)
			{
				for (int j = 0; j < 16; j++)
				{
					if (map[i][j] == 1)
					{
						s1.setPosition(j * 64, i * 64);
						window->draw(s1);
					}
					if (map[i][j] == 2)
					{
						s2.setPosition(j * 64, i * 64);
						window->draw(s2);
					}
				}
			}
			for (int i = 0; i < bullets.size(); i++)
			{
				window->draw(bullets[i].bulletSprite);
			}
			for (int i = 0; i < vectorPlayers.size(); i++)
			{
				vectorPlayers[i].UpdateFlicker(true);
				vectorPlayers[i].draw();
			}
			UpdateFlicker(false);
			draw();
			break;
		}
		case 7:
		{
			window->clear();
			Start[0].setString("Продолжить игру");
			Start[1].setString("Выйти в главное меню");
			for (int i = 0; i < 2; i++)
			{
				Start[i].setFont(font);
				Start[i].setFillColor(Color::White);
				Start[i].setStyle(Text::Bold);
				Start[i].setScale(1.25, 1.25);
				Start[i].setPosition(SCREEN_WIDTH / 2 - Start[i].getGlobalBounds().width / 2, 250 * (i + 1));
			}
			window->waitEvent(event);
			if (event.type == sf::Event::Closed)
			{
				window->close();
			}
			if (Keyboard::isKeyPressed(Keyboard::Escape))
			{
				scene = 6;
			}
			for (int i = 0; i < 2; i++)
			{
				if (Start[i].getGlobalBounds().contains(window->mapPixelToCoords(sf::Mouse::getPosition(*window))))
				{
					if (event.type == event.MouseButtonPressed && event.mouseButton.button == Mouse::Left && i == 0) scene = 6;
					if (event.type == event.MouseButtonPressed && event.mouseButton.button == Mouse::Left && i == 1)
					{
						Packet s;
						s << "DELETEPLAYER" << client.id;
						client.sendPacket(s);
						scene = 0;
					}
					Start[i].setOutlineColor(Color(133, 219, 50));
					Start[i].setOutlineThickness(3);
				}
				else
				{
					Start[i].setOutlineThickness(0);
				}
			}
			for (int i = 0; i < 2; i++)
			{
				window->draw(Start[i]);
			}
			break;
		}
		}
		window->display();
	}
	void load(Texture& texture, Font& font)
	{
		i_player.loadFromFile("tank.jpg");
		i_player.createMaskFromColor(Color::Black);
		t_player.loadFromImage(i_player);
		t_bullet.loadFromFile("bullet.png");
		font.loadFromFile("123.ttf");
		sprite.setTexture(texture);
		sprite.setScale(1, 1); //Размер танка
		//if (!possesed) body.setColor(Color::Red);
		//name = playerName;
		text.setFont(font);
		text.setString(name);
		text.setFillColor(sf::Color::Red);
		text.setPosition(sprite.getGlobalBounds().width / 2 - text.getGlobalBounds().width / 2, sprite.getPosition().y - text.getGlobalBounds().height);
		sprite.setScale(1 / (sprite.getGlobalBounds().width / 64), 1 / (sprite.getGlobalBounds().height / 64));
		soundBuffer.loadFromFile("shoot.wav");
		soundAttack.setBuffer(soundBuffer);
		soundAttack.setVolume(100);
	}
	Player()
	{
		/*for (int i = 0; i < 12; i++)
		{
			for (int j = 0; j < 16; j++)
			{
				if (i == 0 || i == 11 || j == 0 || j == 15) map[i][j] = 1;
				else map[i][j] = 0;
			}
		}*/
		t1.loadFromFile("1.jpg");
		t2.loadFromFile("2.jpg");
		s1.setTexture(t1);
		s2.setTexture(t2);
		//s1.setScale(1 / (s1.getGlobalBounds().width / 64), 1 / (s1.getGlobalBounds().height / 64));
		//s2.setScale(1 / (s2.getGlobalBounds().width / 64), 1 / (s2.getGlobalBounds().height / 64));
		s1.setTextureRect(IntRect(0, 0, 64, 64));
		s2.setTextureRect(IntRect(0, 0, 64, 64));
		i_player.loadFromFile("tank.jpg");
		i_player.createMaskFromColor(Color::Black);
		t_player.loadFromImage(i_player);
		font.loadFromFile("123.ttf");
		sprite.setTexture(t_player);
		//sprite.setScale(1 / (sprite.getGlobalBounds().width / 64), 1 / (sprite.getGlobalBounds().height / 64));
		sprite.setTextureRect(IntRect(0, 0, 64, 64));
		sprite.setOrigin(32, 32);
		text.setFont(font);
		text.setString(name);
		text.setFillColor(sf::Color::White);
		text.setPosition(sprite.getGlobalBounds().width / 2 - text.getGlobalBounds().width / 2, sprite.getPosition().y - text.getGlobalBounds().height);
		soundBuffer.loadFromFile("shoot.wav");
		soundAttack.setBuffer(soundBuffer);
		soundAttack.setVolume(100);
	}
};

int main()
{
	Player player;
	setlocale(0, "");
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console, 15);

	cout << serverIp << endl;
	cout << "Введите имя: ";
	cin >> player.name;

	if (!client.registerOnServer(player.name))
	{
		return 0;
	}

	window = new RenderWindow(VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Client");

	while (window->isOpen())
	{
		player.work();
	}

	client.close();

	return 1;
}