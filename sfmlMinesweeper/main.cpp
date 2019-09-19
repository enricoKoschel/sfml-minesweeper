#include <SFML/Graphics.hpp>
#include <iostream>

using namespace sf;
using namespace std;

////Globale Variablen//
const int cols = 20;	
const int rows = 20;	
const int scl = 40;
int windowWidth = cols * scl;
int windowHeight = (rows * scl) + scl;
int gamestate = 0;		////0 = playing, 1 = paused, 2 = won, 3 = lost//

Text infoText;

////Cell Klasse//
class cell
{
public:
	////Klassen Variablen//
	int x;
	int y;
	int i;
	int j;
	bool mine = false;
	bool revealed = false;
	bool flag = false;
	int neighbourCount;
	int neighbourFlagCount;
	
	
	////Ueberpruepft ob ein gegebener Punkt innerhalb einer Zelle ist//
	bool contains(int x_, int y_, int scl) {
		return (x_ <= x + scl && x_ >= x && y_ <= y + scl && y_ >= y);
	}
	
	////Deckt eine Zelle auf//
	void reveal(cell _grid[cols][rows]) {
		revealed = true;
		if (neighbourCount == 0) {
			floodFill(_grid);
		}
	}

	////Zaehlt die Benachbarten Minen einer Zelle//
	void countNeighbours(cell _grid[cols][rows]) {
		if (mine) {
			neighbourCount = -1;
			return;
		}

		int total = 0;

		for (int xoff = -1; xoff <= 1; xoff++) {
			int celli = i + xoff;
			if (celli < 0 || celli >= cols) continue;

			for (int yoff = -1; yoff <= 1; yoff++) {
				int cellj = j + yoff;
				if (cellj < 0 || cellj >= rows) continue;

				cell neighbour = _grid[celli][cellj];

				if (neighbour.mine) {
					total++;
				}
			}
		}
		neighbourCount = total;
	}

	////Deckt alle Zellen in der Umgebung auf, die einen neighbourCount von 0 haben//
	void floodFill(cell _grid[cols][rows]) {
		for (int xoff = -1; xoff <= 1; xoff++) {
			int celli = i + xoff;
			if (celli < 0 || celli >= cols) continue;

			for (int yoff = -1; yoff <= 1; yoff++) {
				int cellj = j + yoff;
				if (cellj < 0 || cellj >= rows) continue;

				cell neighbour = _grid[celli][cellj];
				
				if (!neighbour.revealed) {
					_grid[celli][cellj].reveal(_grid);
				}
			}
		}
	}

	////Invertiert den Flaggenstatus einer Zelle//
	void switchFlag() {
		flag = !flag;
	}

	////Zaehlt die Benachbarten Flaggen einer Zelle//
	void countFlagNeighbours(cell _grid[cols][rows]) {
		if (flag) {
			neighbourCount = -1;
			return;
		}

		int total = 0;

		for (int xoff = -1; xoff <= 1; xoff++) {
			int celli = i + xoff;
			if (celli < 0 || celli >= cols) continue;

			for (int yoff = -1; yoff <= 1; yoff++) {
				int cellj = j + yoff;
				if (cellj < 0 || cellj >= rows) continue;

				cell neighbour = _grid[celli][cellj];

				if (neighbour.flag) {
					total++;
				}
			}
		}
		neighbourFlagCount = total;
	}

	////Constructor//
	cell(int x_ = 0, int y_ = 0, int i_ = 0, int j_ = 0) {
		x = x_;
		y = y_;
		i = i_;
		j = j_;
	}

	////Destructor//
	~cell() {};
};

////Erstellen des Spielfelds//
cell grid[cols][rows];

////Wird ausgefuehrt wenn der Spieler verloren hat//
void gameover() {
	gamestate = 3;
	for (int i = 0; i <= cols; i++) {
		for (int j = 0; j <= rows; j++) {
			grid[i][j].reveal(grid);
		}
	}
	infoText.setString("Verloren!");
	infoText.setPosition((windowWidth / 2) - (infoText.getGlobalBounds().width / 2), windowHeight - scl);
}

////Wird ausgefuehrt wenn der Spieler gewonnen hat//
void win() {
	gamestate = 2;
	for (int i = 0; i <= cols; i++) {
		for (int j = 0; j <= rows; j++) {
			if (grid[i][j].mine) grid[i][j].flag = true;
			else grid[i][j].reveal(grid);
		}
	}
	infoText.setString("Gewonnen!");
	infoText.setPosition((windowWidth / 2) - (infoText.getGlobalBounds().width / 2), windowHeight - scl);
}

////Main Funktion//
int main(){

	////Lokale Variablen//
	int totalMines;
	int difficulty;
	int safe;
	int revealedCells;
	int frameRate = 60;
	int timerInt = 0;
	bool firstClick = false;
	bool firstClickCheck = false;

	Clock timer;
	Mouse::Button lastPressedButton;
	RectangleShape rect = RectangleShape(Vector2f(scl, scl));

	////Hauptfenster//
	RenderWindow window(VideoMode(windowWidth, windowHeight), "Minesweeper", Style::Close);

	////Framerate//
	window.setFramerateLimit(frameRate);

	////Schriftarten//
	Font fontAlger;
	if (!fontAlger.loadFromFile("../Resources/Fonts/alger.ttf")) {
		return EXIT_FAILURE;
	}

	////Text//
	Text neighbourCountText[cols][rows];
	Text timerText;

	timerText.setFillColor(Color::White);
	timerText.setFont(fontAlger);
	timerText.setPosition(10, windowHeight - scl);
	
	infoText.setFillColor(Color::White);
	infoText.setFont(fontAlger);
	
	////Random seed initialisieren//
	srand(time(NULL));

	////Spielfeld initialisieren//
	for (int i = 0; i <= cols; i++) {
		for (int j = 0; j <= rows; j++) {
			grid[i][j] = cell((i * scl),(j * scl), i, j);
		}
	}
	rect.setOutlineColor(Color(0, 0, 0));
	rect.setOutlineThickness(2);

	////Schwierigkeitsgrad waehlen//
	do {
		system("CLS");
		cout << "Waehle einen Schwierigkeitsgrad.(1 = 20 Minen, 2 = 40 Minen, 3 = 60 Minen, 4 = 80 Minen, 5 = 100 Minen)\n";
		cin >> difficulty;
		cout << endl;
	} while (difficulty > 5 || difficulty < 1);

	switch (difficulty)
	{
	case 1:
		totalMines = 20;
		break;
	case 2:
		totalMines = 40;
		break;
	case 3:
		totalMines = 60;
		break;
	case 4:
		totalMines = 80;
		break;
	case 5:
		totalMines = 100;
		break;
	default:
		break;
	}

	////Minen platzieren//
	{
		int _totalMines = totalMines;
		while (_totalMines > 0) {
			int x = rand() % cols;
			int y = rand() % rows;
			grid[x][y].mine = true;
			_totalMines--;
		}
	}

	////Benachbarte Minen zaehlen//
	for (int i = 0; i < cols; i++) {
		for (int j = 0; j < rows; j++) {
			grid[i][j].countNeighbours(grid);
			neighbourCountText[i][j].setFont(fontAlger);
			neighbourCountText[i][j].setFillColor(Color::Black);
		}
	}

	////Game loop//
	while (window.isOpen()){

		////Maus zuruecksetzen//
		lastPressedButton = Mouse::Button::ButtonCount;

		////Win Bedingungen zuruecksetzen//
		safe = 0;
		revealedCells = 0;

		////Event loop//
		Event event;
		while (window.pollEvent(event)){
			if (event.type == Event::Closed)
				window.close();
			if (event.type == event.MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
					lastPressedButton = Mouse::Left;
			}
			if (event.type == event.MouseButtonReleased && event.mouseButton.button == Mouse::Right) {
				lastPressedButton = Mouse::Right;
			}
		}

		////Clear//
		window.clear();

		////Logik//
		for (int i = 0; i < cols; i++) {
			for (int j = 0; j < rows; j++) {

				////Events bei linker Maustaste//
				if (lastPressedButton == Mouse::Left && grid[i][j].contains(Mouse::getPosition(window).x, Mouse::getPosition(window).y, scl)) {
					if (!grid[i][j].flag) {
						grid[i][j].reveal(grid);

						if (!firstClick) firstClick = true;

						if (grid[i][j].mine && !grid[i][j].flag && gamestate != 2) {
							gameover();
						}
					}

					if (grid[i][j].revealed) {
						grid[i][j].countFlagNeighbours(grid);

						////Automatisches aufdecken bei sicheren Feldern//
						if (grid[i][j].neighbourFlagCount == grid[i][j].neighbourCount) {
							for (int xoff = -1; xoff <= 1; xoff++) {
								int celli = i + xoff;
								if (celli < 0 || celli >= cols) continue;

								for (int yoff = -1; yoff <= 1; yoff++) {
									int cellj = j + yoff;
									if (cellj < 0 || cellj >= rows) continue;

									cell neighbour = grid[celli][cellj];

									if (!neighbour.revealed) {
										grid[celli][cellj].reveal(grid);
									}
									if (grid[celli][cellj].mine && !grid[celli][cellj].flag && gamestate != 2) {
										gameover();
									}
								}
							}
						}
					}
				}

				////Events bei rechter Maustaste//
				if (lastPressedButton == Mouse::Right && grid[i][j].contains(Mouse::getPosition(window).x, Mouse::getPosition(window).y, scl) && !grid[i][j].revealed) {
					grid[i][j].switchFlag();
					if (!firstClick) firstClick = true;
				}

				////Win Bedingungen ueberpruefen//
				if (grid[i][j].mine && grid[i][j].flag) {
					safe++;
				}
				if (!grid[i][j].mine && grid[i][j].flag) {
					safe--;
				}
				if (grid[i][j].revealed) {
					revealedCells++;
				}
				if ((safe == totalMines || revealedCells == (cols * rows) - totalMines) && gamestate != 3) {
					win();
				}

				////Zellen Farben/Position festlegen//
				if (grid[i][j].revealed) {
					rect.setFillColor(Color(220, 220, 220));

					if (grid[i][j].neighbourCount > 0) {
						neighbourCountText[i][j].setPosition(grid[i][j].x + 10, grid[i][j].y);

						neighbourCountText[i][j].setString(to_string(grid[i][j].neighbourCount));
					}

					if (grid[i][j].mine) rect.setFillColor(Color::Red);
				}
				else {
					rect.setFillColor(Color::White);
				}

				if (grid[i][j].flag) rect.setFillColor(Color::Green);

				rect.setPosition(grid[i][j].x, grid[i][j].y);

				////Zellen anzeigen//
				window.draw(rect);
				
				////neighbourCount anzeigen//
				if (grid[i][j].neighbourCount > 0 && grid[i][j].revealed) {
					window.draw(neighbourCountText[i][j]);
				}
			}
		}

		////Timer anzeigen//
		if (firstClick && !firstClickCheck) {
			timer.restart();
			firstClickCheck = true;
		}

		if(gamestate == 0 && firstClick){
			timerInt = timer.getElapsedTime().asSeconds();
		}
		
		int minutes = timerInt / 60;
		int seconds = timerInt % 60;
		string secondsString = seconds < 10 ? "0" + to_string(seconds) : to_string(seconds);
		string minutesString = minutes < 10 ? "0" + to_string(minutes) : to_string(minutes);
		string timerString = minutesString + ":" + secondsString;

		timerText.setString("Zeit: " + timerString);
		window.draw(timerText);

		////Infotext anzeigen//
		window.draw(infoText);

		////Display//
		window.display();
	}

	return EXIT_SUCCESS;
}