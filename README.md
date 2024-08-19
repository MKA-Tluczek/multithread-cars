# Multithreaded Cars
Aplikacja symuluje dwa tory, po których mogą poruszać się pojazdy.

* Pojazdy wjeżdżają na tor okrężny z lewej strony i poruszają się po nim w okręgu ze stałą prędkością.
* Pojazdy wjeżdżają na tor pionowy od góry i poruszają się w dół, przyspieszając.
* Jeżeli na środkowej części pionowego toru znajduje się minimum 1 pojazd, pojazdy na prawym łuku toru okrężnego zatrzymują się, dopóki środkowa część toru pionowego nie będzie ponownie pusta.

Każdy pojazd jest obsługiwany przez osobny wątek. Mechanizm zatrzymywania pojazdów na torze okrężnym zaimplementowany jest poprzez mutexy i zmienne warunkowe:
* Pojazdy na torze pionowym (in/de)krementują licznik podczas poruszania się po odpowiednich sekcjach.
* Każdy dostęp do licznika jest chroniony przez mutex.
* Wątki obsługujące pojazdy na torze okrężnym zatrzymują się, gdy pojazd jest w odpowiedniej sekcji oraz licznik > 0.
* Jeżeli pojazd na torze pionowym, opuszczając środkową część, obniża licznik do wartości 0, wysyła sygnał do wszystkich zatrzymanych pojazdów o wznowienie poruszania się.

## Zależność
Aplikacja używa biblioteki New Curses do wyświetlania torów w oknie konsoli. Podczas kompilacji programu, wymagane jest użycie flagi `-lncurses`.

<sub>Aplikacja wykonana w czerwcu 2024 w ramach kursu "Systemy operacyjne".</sub>
