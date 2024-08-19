#include <ncurses.h>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>
#include <condition_variable>

//zmienne używane do losowości
std::random_device rd;
std::mt19937 gen;

//ile znaków odstępu od krawędzi okna konsoli
const int MARGAIN = 3;

//graniczne wartości x y toru okrężnego oraz x toru kolizyjnego
int circle_left = 0;
int circle_right = 0;
int circle_up = 0;
int circle_down = 0;
int cross_x = 0;

//ile pojazdów jest na wewnętrznej części toru kolizyjnego
int inner_cars_counter = 0;
std::mutex MX_counter; //mutex chroniący powyższy licznik
std::condition_variable stop_circle; //zmienna do blokowania pojazdów okrężnych

//ile symboli na sekundę ruszają sie pojazdy na torze okrężnym
const int CIRCLE_SPEED = 10;

bool end_program = false; //kończy wątki
bool colors = false; //czy konsola ma kolory

int circle_car_x[26];
int circle_car_y[26];
int cross_car_y[26];

int randomInt(int min, int max){
  std::uniform_int_distribution<int> dist(min, max);
  return dist(gen);
}

void printer(){
  while(!end_program){
    if(colors) attron(COLOR_PAIR(1));
    for(int i = 0; i<circle_right; i++) mvprintw(circle_up, i, ".");
    for(int i = circle_up; i<circle_down; i++) mvprintw(i, circle_right, ".");
    for(int i = circle_right; i>circle_left; i--) mvprintw(circle_down, i, ".");
    for(int i = circle_down; i>circle_up; i--) mvprintw(i, circle_left, ".");
    mvprintw((circle_up+circle_down)/2, cross_x+2, "%c", 48+inner_cars_counter);
    if(colors) attroff(COLOR_PAIR(1));
    
    if(colors) attron(COLOR_PAIR(2));
    for(int i = 0; i<=(circle_down+MARGAIN); i++) mvprintw(i, cross_x, ".");
    if(colors) attroff(COLOR_PAIR(2));
    
    if(colors) attron(COLOR_PAIR(3));
    mvprintw(circle_up, cross_x, ".");
    mvprintw(circle_down, cross_x, ".");
    if(colors) attroff(COLOR_PAIR(3));
    
    if(colors) attron(COLOR_PAIR(1));
    for(int i = 0; i<26; i++) mvprintw(circle_car_y[i], circle_car_x[i], "%c", 65+i);
    if(colors) attroff(COLOR_PAIR(1));
    
    if(colors) attron(COLOR_PAIR(2));
    for(int i = 0; i<26; i++) mvprintw(cross_car_y[i], cross_x, "%c", 97+i);
    if(colors) attroff(COLOR_PAIR(2));
    
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void carCircle(int id){
  circle_car_x[id] = 0;
  circle_car_y[id] = circle_up;
  int direction = 0; // 0123 - RDLU
  while(!end_program){
    if(circle_car_x[id] > cross_x){
      std::unique_lock<std::mutex> lock(MX_counter);
      stop_circle.wait(lock, []{return inner_cars_counter == 0;});
    }
    if(direction==0) circle_car_x[id]++;
    if(direction==1) circle_car_y[id]++;
    if(direction==2) circle_car_x[id]--;
    if(direction==3) circle_car_y[id]--;
    if(direction==0 && circle_car_x[id]==circle_right) direction=1;
    if(direction==1 && circle_car_y[id]==circle_down) direction=2;
    if(direction==2 && circle_car_x[id]==circle_left) direction=3;
    if(direction==3 && circle_car_y[id]==circle_up) direction=0;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000/CIRCLE_SPEED));
  }
}

void carCircleMaker(){
  std::thread t[26];
  int delay = 0;
  for(int i = 0; i < 26 && !end_program; i++){
    while(delay > 0 && !end_program){
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      delay--;
    }
    t[i] = std::thread(carCircle, i);
    delay = randomInt(5,100);
  }
  while(!end_program){
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  for(int i = 0; i < 26; i++){
    if(t[i].joinable()) t[i].join();
  }
}

void carCross(int id){
  cross_car_y[id] = 0;
  int next_step_delay = 200;
  int acceleration = randomInt(1, 20);
  while(!end_program && cross_car_y[id] <= (circle_down+MARGAIN)){
    if(cross_car_y[id] == circle_up){
      std::lock_guard<std::mutex> lock(MX_counter);
      inner_cars_counter++;
    }
    cross_car_y[id]++;
    if(cross_car_y[id] == circle_down){
      std::lock_guard<std::mutex> lock(MX_counter);
      inner_cars_counter--;
      if(inner_cars_counter == 0) stop_circle.notify_all();
    }
    next_step_delay -= acceleration;
    std::this_thread::sleep_for(std::chrono::milliseconds(next_step_delay));
  }
  if(cross_car_y[id] > circle_up && cross_car_y[id] < circle_down){
    std::lock_guard<std::mutex> lock(MX_counter);
    inner_cars_counter--;
    if(inner_cars_counter == 0) stop_circle.notify_all();
  }
}

void carCrossMaker(){
  std::thread t[26];
  int delay = 0;
  while(!end_program){
    for(int i = 0; i < 26 && !end_program; i++){
      while(delay > 0 && !end_program){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        delay--;
      }
      if(t[i].joinable()) t[i].join(); //nie nadpisuj aktywnego wątku
      t[i] = std::thread(carCross, i);
      delay = randomInt(5,20);
    }
  }
  for(int i = 0; i < 26; i++){
    if(t[i].joinable()) t[i].join();
  }
}

int main(){
  //init random
  gen = std::mt19937(rd());
  
  //init ncurses
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();
  curs_set(0);
  if(has_colors()){
    colors = true;
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);
    init_pair(3, COLOR_WHITE, COLOR_GREEN);
  }
  
  //init tor
  getmaxyx(stdscr, circle_down, circle_right);
  circle_up = MARGAIN;
  circle_left = MARGAIN;
  circle_down -= (MARGAIN+1);
  circle_right -= (MARGAIN+1);
  cross_x = (circle_right - circle_left) *2 /3 + circle_left;
  
  for(int i = 0; i < 26; i++){
    circle_car_x[i] = -1;
    circle_car_y[i] = -1;
    cross_car_y[i] = -1;
  }
  
  std::thread m1(carCircleMaker);
  std::thread m2(carCrossMaker);
  std::thread p(printer);
  
  while(getch() != 32);
  end_program = true;
  m1.join();
  m2.join();
  p.join();
  endwin();
  
  return 0;
}
